#include "video_decode.h"
#include "ak_thread.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include "queue.h"
#include "ak_mem.h"
#include "ak_vdec.h"
#include "string.h"
#include "leo_api.h"
#include "tuya_ipc_p2p.h"
#include "ring_buffer.h"

#ifdef LINK_LIST_ENABLE
#include "link_list.h"
#endif

static int decode_src_width = 0;
static int decode_src_height = 0;

static bool video_decode_run = false;
static bool decode_switch = false;
static bool video_decode_thread_run = false;
static bool video_decode_ready = false;

struct
{
	int handle_id;
	bool venc_stream_end;
} video_decode = {.handle_id = -1, .venc_stream_end = true};

static char video_decode_type = 0; // 0:h264 1:mjpeg 2:h265

static ak_mutex_t video_decode_mutex;
static ring_buffer video_decode_ring_buffer;

bool video_normal_falg = false;

bool video_decode_queue_reset(void);

#ifdef LINK_LIST_ENABLE

static int consumer = -1;
struct video_vdec_stream
{
	unsigned char *data;			  // stream data
	unsigned int len;				  // stream len in bytes
	unsigned long long ts;			  // timestamp(ms)
	unsigned long seq_no;			  // current stream sequence no according to frame
	enum video_frame_type frame_type; // I or P frame
};

void video_vdec_push(unsigned char *data, int len)
{
	if (data == NULL || len <= 0)
	{
		return;
	}
	struct video_vdec_stream stream;
	memset(&stream, 0, sizeof(struct video_vdec_stream));
	stream.data = data;
	stream.len = len;
	list_node_in(NET_VDEC_LIST, &stream);
}

static int vdec_stream_buf_in(void *dest, void *sour)
{
	struct video_vdec_stream *sour_frame = (struct video_vdec_stream *)sour;
	struct video_vdec_stream *dest_frame = (struct video_vdec_stream *)dest;

	char *pdata = ak_mem_alloc(MODULE_ID_AI, sour_frame->len);
	if (NULL == pdata)
	{
		Debug_Lib("Alloc stream buffer failed!\n");
		return AK_FAILED;
	}

	memcpy((unsigned char *)pdata, sour_frame->data, sour_frame->len);
	dest_frame->data = (unsigned char *)pdata;
	dest_frame->len = sour_frame->len;

	return AK_SUCCESS;
}

static int vdec_stream_buf_out(void **dest, void *sour)
{
	*dest = sour;
	return AK_SUCCESS;
}

static int vdec_stream_buf_release(void *sour)
{
	struct video_vdec_stream *sour_frame = (struct video_vdec_stream *)sour;
	ak_mem_free(sour_frame->data);
	return AK_SUCCESS;
}

static list_operations vdec_fops = {
	.read = vdec_stream_buf_out,
	.wirte = vdec_stream_buf_in,
	.release = vdec_stream_buf_release,
	.data_struct_size = sizeof(struct video_vdec_stream)};
#endif

static void video_decode_device_open(void)
{
	struct ak_vdec_param param;
	memset(&param, 0, sizeof(struct ak_vdec_param));
	param.vdec_type = video_decode_type == 1 ? MJPEG_ENC_TYPE : video_decode_type == 2 ? HEVC_ENC_TYPE
																					   : H264_ENC_TYPE;
	param.output_type = AK_YUV420SP;
	param.sc_width = decode_src_width;
	param.sc_height = decode_src_height;
	param.stream_buf_size = 3 * 1024 * 600 / 2;
	param.frame_buf_num = 4;
	ak_vdec_open(&param, &video_decode.handle_id);
	ak_vdec_clear_buff(video_decode.handle_id);

#ifdef LINK_LIST_ENABLE
	list_init();

	if (list_create(NET_VDEC_LIST, vdec_fops, false) == AK_SUCCESS)
	{
		consumer = list_consumer_register(NET_VDEC_LIST);
		Debug_Lib("list_create NET_VDEC_LIST AK_SUCCESS!!!!\n");
	}
#else
	ring_buffer_init(&video_decode_ring_buffer, 2 * 1024 * 1024, &video_decode_mutex);
#endif
}

static void video_decode_device_close(void)
{

	if (video_decode.handle_id != -1)
	{

		ak_vdec_close(video_decode.handle_id);

		video_decode.handle_id = -1;
		video_decode.venc_stream_end = true;
	}

#ifdef LINK_LIST_ENABLE
	list_dsetroy(NET_VDEC_LIST);
#else
	ring_buffer_release(&video_decode_ring_buffer);
#endif
	video_raw_clear();
}

static void video_data_stream_send(unsigned char *pdata, unsigned int len)
{
	int dec_len = 0;
	unsigned int read_len = len;
	unsigned int send_len = 0;

	extern unsigned long long os_get_ms();
	unsigned long long x = os_get_ms();
	while (read_len > 0)
	{
		ak_vdec_send_stream(video_decode.handle_id, &pdata[send_len], read_len, NONBLOCK, &dec_len);
		read_len -= dec_len;
		send_len += dec_len;
	}
	x = os_get_ms() - x;
	// if (x > 100)
	// printf("ring_buffer_read ms=====>>%llu,len:%d\n", x, len);
}

// static bool snap = true;
static int frame_skip_count = 0;
void frame_skip_count_set(int count)
{
	frame_skip_count = count;
}

static bool video_decode_data_null = true;

bool video_decode_data_status(void)
{
	return video_decode_data_null;
}

static bool frame_skip_enable = false;

bool frame_skip_enable_status(void)
{
	return frame_skip_enable;
}

static void *video_decode_thread_task(void *arg)
{

	extern unsigned long long os_get_ms();
	unsigned long long x = os_get_ms();
	struct ak_vdec_frame frame = {0};
	int count = 0;

	static unsigned long long frame_index = 0;
	while (1)
	{
		memset(&frame, 0, sizeof(struct ak_vdec_frame));
		if (ak_vdec_get_frame(video_decode.handle_id, &frame) == 0)
		{
			unsigned long long u = os_get_ms() - x;
			// if(u >100)

			video_decode_data_null = false;
			count = 0;
			frame_index++;
			// printf("frame_skip_count=====>>%d\n",frame_skip_count);
			if (frame_skip_count && frame_skip_enable)
			{
				frame_skip_count--;
				if (frame_skip_count == 0)
				{
					frame_skip_enable = false;
				}
				ak_vdec_release_frame(video_decode.handle_id, &frame);
			}
			else if ((frame_skip_count == 0) && (u > 45))
			{

				// printf("frame.frame_obj.data.data_size =====>>%d\n", frame.frame_obj.data.data_size);
				// printf("%s ====================>>w:%d      h:%d\n",__func__,frame.width,frame.height);
				x = os_get_ms();
#if 1
				video_raw_lcd_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height, GP_FORMAT_YUV420SP); //,frame.width,frame.height);
				ak_vdec_release_frame(video_decode.handle_id, &frame);
#endif
				// printf("video_raw_lcd_push ms=====>>%lld\n", os_get_ms() - x);
				extern void screen_force_refresh(void);
				screen_force_refresh();
				// video_raw_rec_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height);
			}
			else
			{
				ak_vdec_release_frame(video_decode.handle_id, &frame);
			}

			// 视频稳定的标志位
			if (video_normal_falg == false)
			{
				video_normal_falg = true;
			}
		}
		else if (count++ > 50 && count++ < 100 && video_normal_falg)
		{
			// printf("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFf\n");
			video_normal_falg = false;
		}
		else if ((count++) > 100)
		{
			int status = 0;
			ak_vdec_get_decode_finish(video_decode.handle_id, &status);
			video_decode_data_null = true;

			// printf("===========<<<frame_skip_enable %d>>>===========\n",decode_switch);
			if (status)
			{
				// printf("===========<<<video_decode_data_null %d>>>===========\n",status);
				if (decode_switch)
				{
					if (frame_skip_enable == false)
					{
						frame_skip_enable = true;
						video_raw_clear();
					}
					decode_switch = false;
					// printf("===========<<<frame_skip_enable %d>>>===========\n",status);
				}
				if (decode_switch == false && video_decode_run == false)
				{
					break;
				}
			}
		}
		else
		{
		}
		ak_sleep_ms(1);
	}
	// video_raw_release_all();
	printf("===========<<< video h264 stream stop  >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static void *video_decode_task(void *arg)
{
	video_decode_device_open();
	video_decode_thread_run = true;
	printf("%s============================.>>>\n", __func__);
	ak_pthread_t stream_thread_id;
	video_decode.venc_stream_end = false;
	ak_thread_create(&stream_thread_id, video_decode_thread_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);

	char *buffer = ak_mem_alloc(MODULE_ID_VDEC, 320 * 1024);
	video_decode_ready = true;
	frame_skip_enable = false;
	decode_switch = false;
	frame_skip_count = 0;

	// int xxx = 0;
#ifdef LINK_LIST_ENABLE
	consumer = list_consumer_register(NET_VDEC_LIST);
#endif
	// extern unsigned long long os_get_ms();
	// unsigned long long x = os_get_ms();
	while (video_decode_run == true)
	{
		// ak_thread_mutex_lock(video_decode_ring_buffer.mutex);
		if (!decode_switch)
		{
#ifdef LINK_LIST_ENABLE
			struct video_vdec_stream *stream = NULL;
			while (list_node_out(NET_VDEC_LIST, consumer, (void *)&stream) == AK_SUCCESS)
			{
				// Debug_Lib("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSs:%d\n", xxx);
				xxx++;
				if (stream->len > 0)
				{
					video_data_stream_send(stream->data, stream->len);
				}
			}
			xxx = 0;
#else
			int read_len = ring_buffer_read(&video_decode_ring_buffer, buffer, 320 * 1024);

			// printf("%s============================.>>>%d:%d\n",__func__,decode_switch,read_len);
			if (read_len > 0)
			{
				// 				printf("ring_buffer_read ms=====>>%lld\n",os_get_ms() -x);
				video_data_stream_send((unsigned char *)buffer, read_len);
				// ak_thread_mutex_unlock(video_decode_ring_buffer.mutex);
				// ak_sleep_ms(10);
			}
			else
			{

				// video_raw_rec_push(frame.frame_obj.data.data, 0, frame.width, frame.height, frame.frame_obj.data.pitch_width, frame.frame_obj.data.pitch_height);
				// 视频稳定的标志位
				//  ak_thread_mutex_unlock(video_decode_ring_buffer.mutex);
				//  ak_sleep_ms(10);
			}
			// printf("%d============================.>>>\n",read_len);
#endif
		}
		else
		{
			ak_sleep_ms(1);
		}
	}
	video_decode_ready = false;
	Debug_Lib("+++++++++++\n");
	ak_mem_free(buffer);
	Debug_Lib("+++++++++++\n");

	if (video_decode.venc_stream_end == false)
		ak_vdec_end_stream(video_decode.handle_id);

	Debug_Lib("+++++++++++\n");
	ak_sleep_ms(100);
	ak_thread_join(stream_thread_id);
	Debug_Lib("+++++++++++\n");
	video_decode_device_close();

	video_decode_thread_run = false;
	video_normal_falg = false;
	printf("===========<<<video h264 decode close finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

bool video_decode_wait_thread_status(void)
{
	return video_decode_thread_run;
}

bool video_decode_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (video_decode_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

static ak_pthread_t video_decode_thread = 0;
bool video_decode_open(char type, int src_width, int src_height)
{
	printf("%s============================src_width:%d			src_height:%d\n", __func__, src_width, src_height);
	if (tuya_ipc_get_client_online_num() > 0 && tuya_event_state_get() == TRANS_LIVE_VIDEO_START)
	{
		return false;
	}

	if (video_decode_run == true)
	{
		printf("video decode open after \n");
		return false;
	}

	if (video_decode_wait_thread_quit() == false) // 等待线程退出
	{
		printf("video decode thread wait error \n");
		return false;
	}

	if (src_width == 0 || src_height == 0)
	{
		return false;
	}

	decode_src_width = src_width;
	decode_src_height = src_height;
	video_decode_type = type;

	video_decode_ready = false;
	video_decode_run = true;
	int ret = ak_thread_create(&video_decode_thread, video_decode_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	printf("[%s,%d] ret = %d\n", __func__, __LINE__, ret);

	return true;
}

bool get_video_decode_state(void)
{
	return video_decode_run;
}

bool video_decode_resolution_contrast(int src_width, int src_height)
{
	if (decode_src_width != src_width || decode_src_height != src_height)
		return false;

	return true;
}

void video_decode_init(void)
{
	ak_thread_mutex_init(&video_decode_mutex, NULL);
}

bool video_decode_pause(bool keep)
{
	printf("%s==============>>>%d,%d\n", __func__, video_decode.handle_id, keep);
	if (keep)
	{
		if (video_decode.handle_id != -1 && video_decode.venc_stream_end == false)
		{
			ak_vdec_end_stream(video_decode.handle_id);
			video_decode.venc_stream_end = true;
		}

		decode_switch = true;
		video_decode_queue_reset();
	}
	else
	{
		decode_switch = false;
	}

	return true;
}

bool video_decode_close(void)
{
	if (video_decode_run == false)
	{
		return false;
	}
	video_decode_run = false;

	video_decode_pause(false);
	if (video_decode_thread)
	{
		ak_thread_join(video_decode_thread);
		video_decode_thread = 0;
	}
	return true;
}

bool video_decode_push(char type, unsigned char *data, int len)
{
	if ((video_decode_run == false) || (video_decode_ready == false))
	{
		return false;
	}

	if (type != video_decode_type)
	{
		return false;
	}
	// video_data_stream_send(data, len);

	ring_buffer_write(&video_decode_ring_buffer, (char *)data, len);
	return true;
}

bool video_decode_queue_clear(void)
{
#ifdef LINK_LIST_ENABLE
	list_dsetroy(NET_VDEC_LIST);
#else
	ring_buffer_release(&video_decode_ring_buffer);
#endif
	ak_vdec_clear_buff(video_decode.handle_id); // 在解码清除缓存时不一定清除完全，也许存留些帧作参考，因此下次显示时需要跳过残留帧
	return true;
}
bool video_decode_queue_reset(void)
{
	if ((video_decode_run == false) || (video_decode_ready == false))
	{
		return false;
	}

	video_normal_falg = false;
#ifdef LINK_LIST_ENABLE
	list_dsetroy(NET_VDEC_LIST);
	if (list_create(NET_VDEC_LIST, vdec_fops, false) == AK_SUCCESS)
	{
		consumer = list_consumer_register(NET_VDEC_LIST);
		Debug_Lib("list_create NET_VDEC_LIST AK_SUCCESS!!!!\n");
	}
#else
	ring_buffer_release(&video_decode_ring_buffer);
	// ak_vdec_clear_buff(video_decode.handle_id); //在解码清除缓存时不一定清除完全，也许存留些帧作参考，因此下次显示时需要跳过残留帧
	// video_raw_release_all();
	ring_buffer_init(&video_decode_ring_buffer, 1024 * 1024 * 2, &video_decode_mutex);
#endif
	extern void frame_skip_count_set(int count);
	frame_skip_count_set(5); // 下次显示跳过两帧
	return true;
}

bool get_video_data_display_state(void)
{
	extern INT_T tuya_online_clinet_num_get(void);
	if (tuya_online_clinet_num_get() > 0) // tuya监控就不用管状态-这代码的意义
	{
		// printf("%s ============================>>%d\n",__func__,__LINE__);
		return false;
	}
	return video_normal_falg;
}
