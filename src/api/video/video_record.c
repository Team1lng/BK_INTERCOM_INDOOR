#include "video_decode.h"
#include "ak_common.h"
#include "ak_venc.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "string.h"
#include "ak_common_graphics.h"
#include "avilib.h"
#include "ak_tde.h"
#include <sys/time.h>
#include <netinet/in.h>
#include "queue.h"
#include "leo_api.h"
#include "stdlib.h"
#include "../../include/g711/g711_table.h"
#define AVI_FRAME_BUFFER_MAX 512 * 2

extern void record_video_finnish_event_push(char);

static void (*video_record_finish_func)(const char *path) = NULL;

typedef struct
{
	char file_path[128];
	char recode_mode;
	bool has_audio;
	int width;
	int height;
	bool frame_type; // 0:h264,1:mjpeg 2:h265
	int media_type;
} record_info;

typedef struct
{
	unsigned long long pts;
	unsigned char *data;
	unsigned int len;
} avi_frame_info;

typedef struct
{
	void *prev;
	void *next;

	avi_frame_info frame;
} avi_frame;
static avi_frame avi_frame_buffer[AVI_FRAME_BUFFER_MAX];
static queue_s avi_frame_queue_free;

static queue_s avi_video_queue_head;
static queue_s avi_audio_queue_head;

static ak_mutex_t avi_queue_head_mutex;
static ak_mutex_t avi_queue_free_mutex;

static bool video_record_task_run = false;
static bool video_record_thread_run = false;

static bool video_record_ready = false;

static unsigned long avi_record_node_number = AVI_FRAME_BUFFER_MAX;
static avi_frame *avi_record_queue_node_new(char type, unsigned char *data, int len, bool is_video, unsigned long long pts)
{
	avi_frame *node = NULL;
	ak_thread_mutex_lock(&avi_queue_free_mutex);
	if (queue_empty(&avi_frame_queue_free) == 0) // 队列不为空，有数据
	{
		node = (avi_frame *)queue_delete_next(&avi_frame_queue_free);

		if (node)
		{
			avi_record_node_number--;
		}
	}
	// printf("%s ====================================>>%lu\n",__func__,avi_record_node_number);
	ak_thread_mutex_unlock(&avi_queue_free_mutex);
	if (node == NULL)
	{
		return NULL;
	}
	if (node->frame.data != NULL)
	{
		ak_mem_free(node->frame.data);
	}

	int offset = 0;
	if ((is_video == true) && (type == 0))
	{
		offset = 4;
	}
	node->frame.len = len + offset;
	node->frame.data = (unsigned char *)ak_mem_alloc(MODULE_ID_VENC, node->frame.len);
	if (offset == 4)
	{
		node->frame.data[0] = node->frame.data[1] = node->frame.data[2] = 0;
		node->frame.data[3] = 1;
	}
	memcpy(&node->frame.data[offset], data, len);
	node->frame.pts = get_sys_ms() /* pts */;
	return node;
}

static void avi_record_queue_node_del(avi_frame *node)
{
	if (node != NULL)
	{
		if (node->frame.data != NULL)
		{
			ak_mem_free(node->frame.data);
			node->frame.data = NULL;
		}
		node->frame.len = 0;
		ak_thread_mutex_lock(&avi_queue_free_mutex);
		queue_insert((queue_s *)node, &avi_frame_queue_free);
		avi_record_node_number++;
		// printf("%s ====================================>>%lu\n",__func__,avi_record_node_number);
		ak_thread_mutex_unlock(&avi_queue_free_mutex);
	}
}

#if 0

uint8_t h264_is_keyframe(const unsigned char *buffer, int size)
{

#define SPS_FRAME1 0x07
#define KEY_FRAME2 0x27
#define PPS_FRAME 0x08
#define I_FRAME1 0x05

    uint8_t nal_unit_type = *(buffer) & 0x1F;
    if (nal_unit_type == SPS_FRAME1) // nal_unit_type 的值为 7（二进制表示中的 0b00000111）
    {
        return 2; // SPS 帧
    }
    else if (nal_unit_type == PPS_FRAME) // nal_unit_type 的值为 8（二进制表示中的 0b00001000）
    {
        return 3; // PPS 帧
    }
    else if (nal_unit_type == I_FRAME1) // nal_unit_type 的值为 5（二进制表示中的 0b00000101）
    {
        return 1; // I 帧
    }
    else if (*(buffer) == KEY_FRAME2) {
        return 0xFF; // IDR 帧，也就是关键帧
    }
    return 0;
}
#else
/*
 *	 0:no key frame
 *    1:frame
 * 	 2:sps
 *    3:pps
 */
int h264_is_keyframe(const unsigned char *buffer, int len)
{
#define SPS_FRAME1 0x67
#define KEY_FRAME2 0x27

#define PPS_FRAME 0x68
#define I_FRAME1 0x65
	// #define I_FRAME2   0x21
	//  printf("%2x \n",*buffer);
	if (*(buffer) == SPS_FRAME1) // ox67为 0110 0111(nal_unit_type为低5位，u(5)= 0 0111 = 7)
	{
		return 2;
	}

	else if (*(buffer) == PPS_FRAME) // ox68为 0110 1000 （nal_unit_type为低5位，u(5)= 0 1000 = 8）
	{
		return 3;
	}

	else if (*(buffer) == I_FRAME1) // ox65为 0110 0101 （nal_unit_type为低5位，u(5)= 0 0101 = 5）
	{
		return 1;
	}
	else if (*(buffer) == KEY_FRAME2)
	{
		return 0xFF;
	}
	return 0;

#if 0
	if(!buffer || len < 6)
	{
		return 0;
	}
	unsigned char fragment = (*buffer) & 0x1F;
	unsigned char nal = (*(buffer+1)) & 0x1F;
	if(fragment == 7 || ((fragment == 28 || fragment == 29) && nal == 7)) 
	{
		printf("Got an H264 key frame\n");
		return 1;
	}
	if(fragment == 24) 
	{
		/* May we find an SPS in this STAP-A*/
		buffer++;
		len--;
		uint16_t psize = 0;
		/* We’re reading 3 bytes */
		while(len > 2) 
		{
			memcpy(&psize, buffer, 2);
			psize = ntohs(psize);
			buffer += 2;
			len -= 2;
			int nal =(*buffer) & 0x1F;
			if(nal == 7) 
			{
				printf("Got an SPS/PPS\n");
				return 2;
			}
			buffer += psize;
			len -= psize;
		}
	}
	/* If we got here it’s not a key frame */
	return 0;
#endif
}
#endif

static void avi_write_head(avi_t *handle, double fps, int width, int height, bool has_audio, char h264)
{
	width = 1920;
	height = 1080;
	AVI_set_video(handle, width, height, fps, h264 == 0 ? "H264" : "MJPG");
	// printf("%s=================%d\n\r", __func__, __LINE__);
	if (has_audio == true)
	{
		AVI_set_audio(handle, 1, 16000, 16, 1, 128);
	}
	// printf("%s=================%d\n\r", __func__, __LINE__);
	avi_update_header(handle);
}

static bool avi_write_one_frame(avi_t *handle, const record_info *info)
{
	avi_frame *video_node = NULL;
	avi_frame *audio_node = NULL;

	bool i_frame = false;
	bool sps_frame = false;
	bool pps_frame = false;
	int key_frame = 0;
	printf("[info->has_audio] =======>>>%d\n\r", info->has_audio);
	while (video_record_task_run == true || is_sdcard_insert() == false)
	{
		if (video_node == NULL)
		{
			ak_thread_mutex_lock(&avi_queue_head_mutex);
			if (queue_empty(&avi_video_queue_head) == 0)
			{
				video_node = (avi_frame *)queue_delete_next(&avi_video_queue_head);
			}
			ak_thread_mutex_unlock(&avi_queue_head_mutex);
		}

		if ((info->has_audio == true) && (audio_node == NULL))
		{
			ak_thread_mutex_lock(&avi_queue_head_mutex);
			if (queue_empty(&avi_audio_queue_head) == 0)
			{
				audio_node = (avi_frame *)queue_delete_next(&avi_audio_queue_head);
			}

			ak_thread_mutex_unlock(&avi_queue_head_mutex);
		}

		if ((video_node == NULL) || ((info->frame_type == 0) && ((key_frame = h264_is_keyframe(video_node->frame.data + 4, video_node->frame.len - 4)) == 0)))
		{
			if (video_node != NULL)
			{
				avi_record_queue_node_del(video_node);
				video_node = NULL;
			}
			ak_sleep_ms(10);
			continue;
		}

		if ((info->has_audio == true) && (audio_node == NULL))
		{
			// printf("[receive h264  frame type] ====================>>>>0x%x    [info->has_audio] =======>>>%d\n\r",key_frame,info->has_audio);
			ak_sleep_ms(10);
			continue;
		}
		// printf("[receive h264  frame type] ====================>>>>0x%x    [info->has_audio] =======>>>%d\n\r",key_frame,info->has_audio);

		if ((info->has_audio == false) || (abs(video_node->frame.pts - audio_node->frame.pts) < 100) /*  || 1 */)
		{
			// Debug_Lib("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAa\n\n\n");
			AVI_write_frame(handle, (char *)video_node->frame.data, video_node->frame.len, 1);
			avi_record_queue_node_del(video_node);
			video_node = NULL;
			if (info->frame_type == 0)
			{
				if ((key_frame == 1) && (i_frame == false))
				{
					i_frame = true;
				}
				else if ((key_frame == 2) && (sps_frame == false))
				{
					sps_frame = true;
				}
				else if ((key_frame == 3) && (pps_frame == false))
				{
					pps_frame = true;
				}

				if ((i_frame == true) && (sps_frame == true) && (pps_frame == true))
				{
					if (audio_node != NULL)
					{
						avi_record_queue_node_del(audio_node);
						audio_node = NULL;
					}
					return true;
				}
				else if (key_frame == 0xFF)
				{
					if (audio_node != NULL)
					{
						avi_record_queue_node_del(audio_node);
						audio_node = NULL;
					}
					return true;
				}
			}
			else
			{
				if (audio_node != NULL)
				{
					avi_record_queue_node_del(audio_node);
					audio_node = NULL;
				}
				return true;
			}
		}
		else if (video_node->frame.pts < audio_node->frame.pts)
		{
			// Debug_Lib("LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL:%d\n\n\n",abs(video_node->frame.pts - audio_node->frame.pts));
			avi_record_queue_node_del(video_node);
			video_node = NULL;
		}
		else
		{
			// Debug_Lib("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGg:%d\n\n\n",abs(video_node->frame.pts - audio_node->frame.pts));
			avi_record_queue_node_del(audio_node);
			audio_node = NULL;
		}
	}

	if (video_node != NULL)
	{
		avi_record_queue_node_del(video_node);
		video_node = NULL;
	}

	if (audio_node != NULL)
	{
		avi_record_queue_node_del(audio_node);
		audio_node = NULL;
	}

	video_record_task_run = false;
	return false;
}

static void avi_write_frame_loop(avi_t *handle, const record_info *info, unsigned int *frame_total)
{

	avi_frame *video_node = NULL;
	avi_frame *audio_node = NULL;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	// unsigned long long cur_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	while (video_record_task_run == true)
	{
		if (video_node == NULL)
		{
			ak_thread_mutex_lock(&avi_queue_head_mutex);
			if (queue_empty(&avi_video_queue_head) == 0)
			{
				video_node = (avi_frame *)queue_delete_next(&avi_video_queue_head);
			}
			ak_thread_mutex_unlock(&avi_queue_head_mutex);
		}

		if ((info->has_audio == true) && (audio_node == NULL))
		{
			ak_thread_mutex_lock(&avi_queue_head_mutex);
			if (queue_empty(&avi_audio_queue_head) == 0)
			{
				audio_node = (avi_frame *)queue_delete_next(&avi_audio_queue_head);
			}
			ak_thread_mutex_unlock(&avi_queue_head_mutex);
		}

		if ((video_node == NULL) /* || (video_node->frame.pts < cur_ms) */)
		{
			/* 			if (video_node != NULL)
						{
							printf("%s=====================%d\n",__func__,__LINE__);
							avi_record_queue_node_del(video_node);
							video_node = NULL;
						} */
			ak_sleep_ms(10);
			continue;
		}

		if (is_sdcard_insert() == false)
		{
			printf("avi_write_frame Error : Abnormal SD card.\n");
			break;
		}
		/*
		*如果video_pts > audio_pts :音频在后，视频在前。则需要直接写入音频帧。
		 如果video_pts < audio_pts :音频在前，视频在后，则直接将过滤掉视频帧，直到获取到大于音频帧的数据*/
		if ((audio_node != NULL) && (video_node->frame.pts > audio_node->frame.pts))
		{
			// printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaA.\n");
			int pcm_buffer_size = audio_node->frame.len * 2;
			char *pcm_buffer = NULL;
			pcm_buffer = malloc(pcm_buffer_size);
			if (pcm_buffer == NULL)
			{
				printf("Error while allocating memory for write buffer.\n");
				continue;
			}
			alaw_to_pcm16(audio_node->frame.len, (const char *)audio_node->frame.data, pcm_buffer);
			AVI_write_audio(handle, (char *)pcm_buffer, pcm_buffer_size);
			avi_record_queue_node_del(audio_node);
			audio_node = NULL;
			free(pcm_buffer);
			pcm_buffer = NULL;
		}
		else if (video_node != NULL)
		{
			// printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb.\n");
			AVI_write_frame(handle, (char *)video_node->frame.data, video_node->frame.len, h264_is_keyframe(video_node->frame.data + 4, video_node->frame.len - 1));
			avi_record_queue_node_del(video_node);
			video_node = NULL;

			(*frame_total)++;
		}
		ak_sleep_ms(1);
	}

	video_record_task_run = false;

	if (video_node != NULL)
	{
		avi_record_queue_node_del(video_node);
		video_node = NULL;
	}

	if (audio_node != NULL)
	{
		avi_record_queue_node_del(audio_node);
		audio_node = NULL;
	}
}

static void avi_frame_release_all(void)
{
	while (!queue_empty(&avi_video_queue_head))
	{
		avi_frame *node = (avi_frame *)queue_delete_next(&avi_video_queue_head);
		avi_record_queue_node_del(node);
	}

	while (!queue_empty(&avi_audio_queue_head))
	{
		avi_frame *node = (avi_frame *)queue_delete_next(&avi_audio_queue_head);
		avi_record_queue_node_del(node);
	}
}

static void *video_record_task(void *arg)
{
	unsigned long avi_write_frame_duration = 0;
	unsigned int avi_write_frame_count = 0;
	alaw_pcm16_tableinit();
	record_info *info = (record_info *)arg;

	char temp_file[128] = {0};
	sprintf(temp_file, "%stemp", info->file_path);
	avi_t *avi_handle = AVI_open_output_file(temp_file);
	printf("AVI_open_output_file :%p\n\r", avi_handle);
	if (avi_handle == NULL)
	{
		goto finish;
	}
	avi_write_head(avi_handle, 30.0, info->width, info->height, info->has_audio, info->frame_type);

	video_record_ready = true;

	printf("\n\n\nEncode to AVI Start. avi_record_node_number:%lu  \n\n\n\n", avi_record_node_number);
	if (avi_write_one_frame(avi_handle, info) == true)
	{
		struct timeval start_tv;
		gettimeofday(&start_tv, NULL);
		printf("encode avi start... \n");
		avi_write_frame_loop(avi_handle, info, &avi_write_frame_count);

		struct timeval end_tv;
		gettimeofday(&end_tv, NULL);

		avi_write_frame_duration = end_tv.tv_sec * 1000 + end_tv.tv_usec / 1000 - start_tv.tv_sec * 1000 - start_tv.tv_usec / 1000;
	}
	ak_thread_mutex_lock(&avi_queue_head_mutex);
	video_record_ready = false;

	avi_handle->fps = 1000 * avi_write_frame_count / (avi_write_frame_duration + 0.1);
	printf("\n\n\nEncode to AVI Finish. video frame:%lffps drution:%lums  \n\n\n\n", avi_handle->fps, avi_write_frame_duration);
	printf("\n\n\nEncode to AVI Finish. avi_record_node_number:%lu  \n\n\n\n", avi_record_node_number);
	AVI_close(avi_handle);

	avi_frame_release_all();
	ak_thread_mutex_unlock(&avi_queue_head_mutex);

finish:

	if (avi_write_frame_count > 40)
	{
		if (video_record_finish_func != NULL)
			video_record_finish_func(temp_file);
	}
	if (is_sdcard_insert() == false)
	{
		video_record_thread_run = false;
		video_record_task_run = false;
		ak_thread_exit();
		return NULL;
	}
	/******************
	校验视频是否错误
	*******************/
	// 删除临时文件
	if (access(temp_file, F_OK) == 0)
	{
		remove(temp_file);
	}

	extern void detect_sd_free_space(void);
	detect_sd_free_space();
	// extern int media_bad_path_check(const char* file,char mode);
	// media_bad_path_check(info->file_path,info->media_type);
	printf("%s =================>>%s::%d\n\r", __func__, info->file_path, avi_write_frame_count);
	/******************
	同步记录的视频
	******************/
	system("sync");

	video_record_thread_run = false;
	video_record_task_run = false;
	record_video_finnish_event_push(info->recode_mode);
	ak_thread_exit();
	return NULL;
}

static record_info record_info1 = {{0}, 0};

bool video_record_start(const char *path, bool has_audio, int width, int height, char type, char record_mode, int media_type, void (*finish_callback)(const char *path))
{
	if ((video_record_task_run == true) || (video_record_thread_run == true))
	{
		printf("video_record_task_run %d     video_record_thread_run:%d\n", video_record_task_run, video_record_thread_run);
		return false;
	}
	printf("video_record_start need audio : %d\n\r", has_audio);
	strcpy(record_info1.file_path, path);
	record_info1.has_audio = has_audio;
	record_info1.frame_type = type;
	record_info1.width = width;
	record_info1.height = height;
	record_info1.recode_mode = record_mode;
	record_info1.media_type = media_type;
	video_record_task_run = true;
	video_record_thread_run = true;
	video_record_finish_func = finish_callback;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, video_record_task, &record_info1, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool video_record_stop(void)
{
	if (video_record_task_run == false)
	{
		return true;
	}

	video_record_task_run = false;
	return true;
}

bool video_record_data_push(record_data_node *record_node)
{
	ak_thread_mutex_lock(&avi_queue_head_mutex);
	if (video_record_ready == false)
	{
		ak_thread_mutex_unlock(&avi_queue_head_mutex);
		return false;
	}

	if (record_node->is_video == true)
	{
		avi_frame *node = avi_record_queue_node_new(record_node->type, (record_node->data) + 4, record_node->len - 4, record_node->is_video, record_node->pts);
		if (node == NULL)
		{
			printf("record video full\n\r");
			ak_thread_mutex_unlock(&avi_queue_head_mutex);
			return false;
		}
		// printf("record video queue_insert===================>>>>\n\r");
		queue_insert((queue_s *)node, &avi_video_queue_head);
	}
	else
	{
		if (record_info1.has_audio == true)
		{
			avi_frame *node = avi_record_queue_node_new(record_node->type, record_node->data, record_node->len, record_node->is_video, record_node->pts);
			if (node == NULL)
			{
				printf("record audio full\n\r");
				ak_thread_mutex_unlock(&avi_queue_head_mutex);
				return false;
			}
			// printf("record audio queue_insert===================>>>>\n\r");
			queue_insert((queue_s *)node, &avi_audio_queue_head);
		}
	}
	ak_thread_mutex_unlock(&avi_queue_head_mutex);
	return true;
}

void video_record_init(void)
{
	ak_thread_mutex_init(&avi_queue_head_mutex, NULL);
	ak_thread_mutex_init(&avi_queue_free_mutex, NULL);

	queue_initialize(&avi_video_queue_head);
	queue_initialize(&avi_audio_queue_head);
	queue_initialize(&avi_frame_queue_free);

	for (int i = 0; i < AVI_FRAME_BUFFER_MAX; i++)
	{
		queue_insert((queue_s *)&avi_frame_buffer[i], &avi_frame_queue_free);
	}
}

bool is_video_recording(void)
{
	return video_record_thread_run;
}

void wait_video_record_finish(void)
{
	int cont = 300;
	while (cont--)
	{
		if (is_video_recording() == false)
		{
			break;
		}
		ak_sleep_ms(1);
	}
}