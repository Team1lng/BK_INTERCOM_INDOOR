#include "avilib.h"
#include "ak_thread.h"
#include "stdio.h"
#include "ak_mem.h"
#include "ak_ao.h"
#include "queue.h"
#include "stdbool.h"
#include "ak_vdec.h"
#include "video_decode.h"
#include "leo_api.h"
#include "audio_output.h"
// #define AVI_BUFFER_MAX 1024*1024
#define AVI_AUDIO_FRAME_SIZE 4096 // 1024
#define VIDEO_PLAY_QUEUE_MAX 5

static bool video_play_task_run = false;
static bool video_play_thread_run = false;
static bool video_play_flag1 = false;
static bool video_play_flag2 = false;
static char video_play_file_path[64] = {0};
static ak_pthread_t thread_id;

extern bool media_thumb_decode_clear(int handle);
extern void audio_output_restart(void);

typedef struct
{
	void *prev;
	void *next;

	unsigned char *data;
	unsigned int len;
} video_play_queue;

typedef struct
{

	bool run;
	int handle_id;

	ak_pthread_t thread_id;

} video_play_info;

static ak_mutex_t video_play_video_head_mutex;
static ak_mutex_t video_play_video_free_mutex;

static ak_mutex_t video_play_audio_head_mutex;
static ak_mutex_t video_play_audio_free_mutex;

static queue_s video_play_video_queue_head;
static queue_s video_play_audio_queue_head;

static queue_s video_play_video_queue_free;
static queue_s video_play_audio_queue_free;

static video_play_queue video_play_video_queue_buffer[VIDEO_PLAY_QUEUE_MAX];
static video_play_queue video_play_audio_queue_buffer[VIDEO_PLAY_QUEUE_MAX];

static int video_play_video_frame_index = 0;
static int video_play_audio_frame_index = 0;
static int video_play_video_frame_total = 0;
static double video_play_video_frame_duration = 0;

static bool video_play_video_frame_eof = false;

static unsigned long long video_play_clock_base = 0;
static unsigned int video_index = 0;
#define PLAY_VIDEO_STATE_IDLE 0X00
#define PLAY_VIDEO_STATE_PLAY 0X02
#define PLAY_VIDEO_STATE_PAUSE 0X03
static unsigned char video_play_state = PLAY_VIDEO_STATE_IDLE;

static video_play_queue *video_play_queue_node_new(char type, int size)
{
	queue_s *queue_free = type == 0 ? (&video_play_video_queue_free) : (&video_play_audio_queue_free);

	ak_mutex_t *pmutex = type == 0 ? (&video_play_video_free_mutex) : (&video_play_audio_free_mutex);

	video_play_queue *node = NULL;
	ak_thread_mutex_lock(pmutex);
	if (queue_empty(queue_free) == 0)
	{
		node = (video_play_queue *)queue_delete_next(queue_free);
	}
	ak_thread_mutex_unlock(pmutex);
	if (node == NULL)
	{
		return NULL;
	}

	if (node->data != NULL)
	{
		ak_mem_free(node->data);
	}
	node->data = ak_mem_alloc(MODULE_ID_VDEC, size);
	node->len = size;
	return node;
}

static void video_play_queue_node_del(char type, video_play_queue *node)
{
	if (node != NULL)
	{
		if (node->data != NULL)
		{
			ak_mem_free(node->data);
			node->data = NULL;
		}

		queue_s *queue_free = type == 0 ? (&video_play_video_queue_free) : (&video_play_audio_queue_free);

		ak_mutex_t *pmutex = type == 0 ? (&video_play_video_free_mutex) : (&video_play_audio_free_mutex);

		ak_thread_mutex_lock(pmutex);
		queue_insert((queue_s *)node, queue_free);
		ak_thread_mutex_unlock(pmutex);
	}
}

bool video_play_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (video_play_thread_run == false)
		{
			return true;
		}
		usleep(10 * 1000);
	}
	return false;
}

static double video_frame_duration_get(avi_t *handle)
{
	return (double)1000 / (AVI_frame_rate(handle) == 0 ? AVI_frame_rate(handle) + 0.1 : AVI_frame_rate(handle));
}

static double audio_frame_duration_get(long sample, int ch, int byte)
{
	return (AVI_AUDIO_FRAME_SIZE /* * 8  */ * 1000.0) / (sample * byte * ch /* + 0.0001 */);
}

#if 1
extern void screen_force_refresh(void);
static void *video_play_decode_frame_task(void *arg)
{
	video_play_info *frame_info = (video_play_info *)arg;

	struct ak_vdec_frame frame;
	while (frame_info->run == true)
	{
		memset(&frame, 0, sizeof(struct ak_vdec_frame));
		if (ak_vdec_get_frame(frame_info->handle_id, &frame) == 0)
		{
			// printf("video_play_video_frame_index%d\n\r",video_play_video_frame_index);
			if (video_play_video_frame_index == 0)
			{
				video_play_clock_base = get_sys_ms();
				Debug_Lib("video_play_clock_base:%llu,video_frame_duration:%lf\n\r", video_play_clock_base, video_play_video_frame_duration);
			}
			unsigned long long v_pts = video_play_clock_base + (unsigned long long)(video_play_video_frame_index * video_play_video_frame_duration);
			video_play_video_frame_index++;

			unsigned long long cur_pts = get_sys_ms();
			if (((cur_pts >= v_pts) && ((cur_pts - v_pts) < (unsigned int)video_play_video_frame_duration)))
			{
				// printf("AAAAAAAAAAAAAAAAAAAAA fast :%d,cur_pts:%llu,v_pts:%llu,abs:%d\n\r\n\r",video_play_video_frame_index,cur_pts,v_pts,abs(cur_pts-v_pts));
				video_raw_lcd_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height, GP_FORMAT_YUV420SP);
				screen_force_refresh();
			}
			else if (cur_pts < v_pts)
			{
				ak_sleep_ms(v_pts - cur_pts);
				// printf("VVVVVVVVVVVVVVVVVVVVVV fast :%d,cur_pts:%llu,v_pts:%llu,abs:%d,get_sys_ms:%llu\n\r",video_play_video_frame_index,cur_pts,v_pts,abs(cur_pts-v_pts),get_sys_ms());
				video_raw_lcd_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height, GP_FORMAT_YUV420SP);
				screen_force_refresh();
			}
			else
			{
				// printf("skip play video frame :%d,cur_pts:%llu,v_pts:%llu,abs:%d,video_play_audio_frame_index:%d\n\r", video_play_video_frame_index, cur_pts, v_pts, abs(cur_pts - v_pts), video_play_audio_frame_index);
			}
			ak_vdec_release_frame(frame_info->handle_id, &frame);
			if (video_play_video_frame_index == 1)
			{
				extern void fb_video_mode_enable(bool);
				fb_video_mode_enable(true);
			}
		}
		ak_sleep_ms(1);
	}
	printf("===========<<< video play display thread finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

#endif

static void *video_play_decode_video_task(void *arg)
{
	video_play_info *decode_info = (video_play_info *)arg;

#if 1
	video_play_info frame_info;
	frame_info.handle_id = decode_info->handle_id;
	frame_info.run = true;
	ak_thread_create(&frame_info.thread_id, video_play_decode_frame_task, &frame_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
#endif

	//	struct ak_vdec_frame frame;
	while (decode_info->run)
	{
		video_play_queue *node = NULL;
		ak_thread_mutex_lock(&video_play_video_head_mutex);
		if (queue_empty(&video_play_video_queue_head) == 0)
		{
			node = (video_play_queue *)queue_delete_next(&video_play_video_queue_head);
			video_index--;
		}
		ak_thread_mutex_unlock(&video_play_video_head_mutex);
		if (node != NULL)
		{
			// printf("video_index%d\n\r",video_index);
			extern void thumb_stream_send(int, unsigned char *, int);
			thumb_stream_send(decode_info->handle_id, node->data, node->len);
			video_play_queue_node_del(0, node);
		}

		ak_sleep_ms(1);
	}

#if 1
	frame_info.run = false;
	ak_thread_join(frame_info.thread_id);
#endif
	media_thumb_decode_clear(decode_info->handle_id);
	printf("===========<<< video frame thread finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool video_play_audio_buffer_wait_clear(int threshold)
{
	// extern bool audio_output_buffer_get(int *, int *);

#if 1
	audio_output_restart();
#else
	/* 下面这段代码偶尔出现缓存超标情况，导致不能正常退出循环 */
	int total, remain;
	while (1)
	{
		if (audio_output_buffer_get(&total, &remain) == false)
		{
			return false;
		}

		if (remain <= threshold)
		{
			break;
		}
	}
#endif
	return true;
}

static bool audio_decode_open_finish = false;
static void *video_play_audio_decode_task(void *arg)
{
	video_play_info *decode_info = (video_play_info *)arg;

	double frame_duration = audio_frame_duration_get(AK_AUDIO_SAMPLE_RATE_16000, 1, 2);
	printf("audio play duration:%lf \n", frame_duration);

	audio_decode_open_finish = true;
	while (decode_info->run == true)
	{
		if (video_play_state == PLAY_VIDEO_STATE_PAUSE)
		{
			ak_sleep_ms(10);
			continue;
		}
		video_play_queue *node = NULL;
		ak_thread_mutex_lock(&video_play_audio_head_mutex);
		if (queue_empty(&video_play_audio_queue_head) == 0)
		{
			node = (video_play_queue *)queue_delete_next(&video_play_audio_queue_head);
		}
		ak_thread_mutex_unlock(&video_play_audio_head_mutex);

		if (node != NULL)
		{
			//  extern bool audio_output_write(unsigned char *, int);
			if (audio_ring_volume_get() != 100)
			{
				audio_ring_volume_set(100);
			}

			if (video_play_audio_frame_index == 0)
			{
				video_play_clock_base = get_sys_ms() /*  - video_play_audio_frame_index*frame_duration */;
			}
			unsigned long long cur_pts = get_sys_ms();
			unsigned long long a_pts = video_play_clock_base + (unsigned long long)(video_play_audio_frame_index * frame_duration);
			if (cur_pts < a_pts && (a_pts - cur_pts) > (unsigned int)frame_duration)
			{
				// Debug_Lib("快快 fast :%d,cur_pts:%llu,a_pts:%llu,abs:%d,get_sys_ms:%llu\n\r",video_play_video_frame_index,cur_pts,a_pts,abs(cur_pts-a_pts),get_sys_ms());
				ak_sleep_ms((a_pts - cur_pts) / 3 * 2);
				audio_output_write(node->data, node->len, false, NULL);
			}
			else
			{
				// if(cur_pts < a_pts)
				// Debug_Lib("慢 fast :%d,cur_pts:%llu,a_pts:%llu,abs:%d,get_sys_ms:%llu\n\r",video_play_video_frame_index,cur_pts,a_pts,abs(cur_pts-a_pts),get_sys_ms());
				// else
				// Debug_Lib("快 fast :%d,cur_pts:%llu,a_pts:%llu,abs:%d,get_sys_ms:%llu\n\r",video_play_video_frame_index,cur_pts,a_pts,abs(cur_pts-a_pts),get_sys_ms());
				audio_output_write(node->data, node->len, false, NULL);
			}
			video_play_queue_node_del(1, node);
			video_play_audio_frame_index++;
		}

		ak_sleep_ms(1);
	}
	printf("===========<<< audio frame thread finish >>>===========\n");
	audio_output_restart();
	audio_decode_open_finish = false;
	ak_thread_exit();
	return NULL;
}

static void video_play_queue_realease(void)
{
	ak_thread_mutex_lock(&video_play_video_head_mutex);
	while (!queue_empty(&video_play_video_queue_head))
	{
		video_play_queue *node = (video_play_queue *)queue_delete_next(&video_play_video_queue_head);
		if (node != NULL)
		{
			video_play_queue_node_del(0, node);
		}
	}
	ak_thread_mutex_unlock(&video_play_video_head_mutex);

	ak_thread_mutex_lock(&video_play_audio_head_mutex);
	while (!queue_empty(&video_play_audio_queue_head))
	{
		video_play_queue *node = (video_play_queue *)queue_delete_next(&video_play_audio_queue_head);
		if (node != NULL)
		{
			video_play_queue_node_del(1, node);
		}
	}
	ak_thread_mutex_unlock(&video_play_audio_head_mutex);

	// video_raw_release_all();
}

static void *video_play_task(void *arg)
{
	video_play_thread_run = true;
	video_play_flag1 = true;
	video_play_flag2 = true;
	avi_t *avi_handle = AVI_open_input_file(video_play_file_path, 1);

	video_play_info video_info = {
		.handle_id = -1,
		.run = false,
	};
	video_play_info audio_info = {
		.handle_id = -1,
		.run = false,
	};

	extern int media_h264_decode_handle_id_get(void);
	media_thumb_decode_clear(media_h264_decode_handle_id_get());

	char *format = AVI_video_compressor(avi_handle);
	if (strcmp(format, "H264") == 0)
	{
		extern int media_h264_decode_handle_id_get(void);
		video_info.handle_id = media_h264_decode_handle_id_get();
	}
	else if (strcmp(format, "MJPG") == 0)
	{
		extern int media_mjpeg_decode_handle_id_get(void);
		video_info.handle_id = media_mjpeg_decode_handle_id_get();
	}

	video_play_video_frame_eof = false;
	video_play_video_frame_index = 0;
	video_play_audio_frame_index = 0;
	video_play_video_frame_total = AVI_video_frames(avi_handle);
	video_play_video_frame_duration = (double)video_frame_duration_get(avi_handle);
	printf("video frame duration:%lf ms    video_play_video_frame_total:%d\n\r", video_play_video_frame_duration, video_play_video_frame_total);
	if (video_info.handle_id != -1)
	{
		video_info.run = true;
		ak_thread_create(&video_info.thread_id, video_play_decode_video_task, &video_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	}

	int audio_channels = AVI_audio_channels(avi_handle);
	int audio_rate = AVI_audio_rate(avi_handle);
	int audio_byte = AVI_audio_bits(avi_handle);
	printf("channels:%d rate:%d byte:%d \n", audio_channels, audio_rate, audio_byte);
	if ((audio_channels == 1) && (audio_rate == 16000) && (audio_byte == 16))
	{
		audio_info.run = true;

		video_play_audio_buffer_wait_clear(8000);
		printf("video_play_audio_decode_task:%d \n", __LINE__);

		// extern bool audio_output_open(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate, int vol, int gain);
		audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000, 60, 5);
		printf("video_play_audio_decode_task:%d \n", __LINE__);
		ak_thread_create(&audio_info.thread_id, video_play_audio_decode_task, &audio_info, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
		int timeout = 0;
		while (audio_decode_open_finish == false)
		{
			if (timeout++ > 1000)
			{
				break;
			}
			ak_sleep_ms(1);
		}
	}

	/*用于判断帧是否已经取完*/
	int video_get_frame_index = 0;

	int data_len = 0;
	int key = 0;

	Debug_Lib("**************************PLAY_VIDEO_STATE_PLAY**************************\n");
	video_play_state = PLAY_VIDEO_STATE_PLAY;

	while (video_play_task_run == true)
	{
		if (video_play_state == PLAY_VIDEO_STATE_PAUSE)
		{
			ak_sleep_ms(10);
			continue;
		}
		if (video_get_frame_index < video_play_video_frame_total)
		{
			long video_frame_size = AVI_cur_frame_size(avi_handle);
			if (video_frame_size > 0)
			{

				video_play_queue *node = video_play_queue_node_new(0, video_frame_size);
				if (node != NULL)
				{
					if ((data_len = (int)AVI_read_frame(avi_handle, (char *)node->data, &key)) > 0)
					{
						node->len = data_len;
						ak_thread_mutex_lock(&video_play_video_head_mutex);
						queue_insert((queue_s *)node, &video_play_video_queue_head);
						video_index++;
						ak_thread_mutex_unlock(&video_play_video_head_mutex);
						// printf("video_index=========>>>%d\n\r",video_index);
						video_get_frame_index++;
					}
					else
					{
						video_play_queue_node_del(0, node);
					}
				}
			}
		}

		if (audio_info.run == true)
		{
			video_play_queue *node = video_play_queue_node_new(1, AVI_AUDIO_FRAME_SIZE);
			if (node != NULL)
			{
				if ((data_len = (int)AVI_read_audio(avi_handle, (char *)node->data, AVI_AUDIO_FRAME_SIZE)) > 0)
				{
					node->len = data_len;
					ak_thread_mutex_lock(&video_play_audio_head_mutex);
					queue_insert((queue_s *)node, &video_play_audio_queue_head);
					ak_thread_mutex_unlock(&video_play_audio_head_mutex);
				}
				else
				{
					video_play_queue_node_del(1, node);
				}
			}
		}

		if ((video_get_frame_index >= video_play_video_frame_total) && (queue_empty(&video_play_video_queue_head)) && (queue_empty(&video_play_audio_queue_head)))
		{
			video_play_video_frame_eof = true;
			video_play_state = PLAY_VIDEO_STATE_PAUSE;
			video_play_video_frame_index = 0;
			video_play_audio_frame_index = 0;
			video_get_frame_index = 0;

			AVI_set_video_position(avi_handle, 0);
			AVI_set_audio_position(avi_handle, 0);
			printf("video play eof \n\r");
		}

		ak_sleep_ms(1);
	}

	if (video_info.run == true)
	{
		video_info.run = false;
		printf("===========<<< wati video play finish >>>===========\n");
		ak_thread_join(video_info.thread_id);
		video_play_flag1 = false;
	}

	if (audio_info.run == true)
	{

		audio_info.run = false;

		printf("===========<<< wati audio play finish >>>===========\n");
		ak_thread_join(audio_info.thread_id);
		video_play_flag2 = false;
	}

	AVI_close(avi_handle);
	video_play_queue_realease();
	video_play_thread_run = false;

	printf("===========<<< video play finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

void video_play_init(void)
{
	queue_initialize(&video_play_video_queue_head);
	queue_initialize(&video_play_audio_queue_head);

	queue_initialize(&video_play_video_queue_free);
	queue_initialize(&video_play_audio_queue_free);

	memset(video_play_video_queue_buffer, 0, sizeof(video_play_queue) * VIDEO_PLAY_QUEUE_MAX);
	memset(video_play_audio_queue_buffer, 0, sizeof(video_play_queue) * VIDEO_PLAY_QUEUE_MAX);
	for (int i = 0; i < VIDEO_PLAY_QUEUE_MAX; i++)
	{
		queue_insert((queue_s *)&video_play_video_queue_buffer[i], &video_play_video_queue_free);
		queue_insert((queue_s *)&video_play_audio_queue_buffer[i], &video_play_audio_queue_free);
	}

	ak_thread_mutex_init(&video_play_video_head_mutex, NULL);
	ak_thread_mutex_init(&video_play_audio_head_mutex, NULL);

	ak_thread_mutex_init(&video_play_video_free_mutex, NULL);
	ak_thread_mutex_init(&video_play_audio_free_mutex, NULL);
}

bool video_play_open(const char *file)
{
	Debug_Lib("%d %d\n", video_play_flag1, video_play_flag2);
	if (video_play_task_run == true)
	{
		printf("video play thread working \n");
		return false;
	}

	if (video_play_wait_thread_quit() == false)
	{
		printf("video play thread wait fail \n");
		return false;
	}

	memset(video_play_file_path, 0, sizeof(video_play_file_path));
	strcpy(video_play_file_path, file);

	video_play_task_run = true;
	ak_thread_create(&thread_id, video_play_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	return true;
}

bool video_play_stop(void)
{
	if (video_play_task_run == false)
	{
		return false;
	}
	video_play_task_run = false;
	ak_thread_join(thread_id);
	video_play_state = PLAY_VIDEO_STATE_IDLE;
	return true;
}

bool video_play_pause(void)
{
	if (video_play_state == PLAY_VIDEO_STATE_IDLE)
	{
		return false;
	}

	if (video_play_state == PLAY_VIDEO_STATE_PAUSE)
	{
		video_play_video_frame_eof = false;
		video_play_clock_base = get_sys_ms() - (unsigned long long)(video_play_video_frame_index * video_play_video_frame_duration);
		video_play_state = PLAY_VIDEO_STATE_PLAY;
	}
	else if (video_play_state == PLAY_VIDEO_STATE_PLAY)
	{
		video_play_state = PLAY_VIDEO_STATE_PAUSE;
	}
	return true;
}

char video_play_get_status(void)
{
	return video_play_state == PLAY_VIDEO_STATE_PLAY ? 1 : video_play_state == PLAY_VIDEO_STATE_PAUSE ? 2
																									  : 0;
}

bool video_play_duration_get(int *cur, int *total)
{
	if (video_play_state == PLAY_VIDEO_STATE_IDLE)
	{
		return false;
	}

	*total = (unsigned long long)(video_play_video_frame_total * video_play_video_frame_duration);
	if (video_play_video_frame_eof == true)
	{
		*cur = *total;
	}
	else
	{
		*cur = (unsigned long long)(video_play_video_frame_index * video_play_video_frame_duration);
	}
	return true;
}
