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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "lvgl.h"
#include "avilib.h"

static int media_mjpeg_handle_id = -1;
static int media_h264_handle_id = -1;
static int media_thumb_width = 0;
static int media_thumb_height = 0;

static bool madia_thumb_task_run = false;
static bool media_thumb_thread = false;

static bool media_decode_device_ready = false;

static ak_mutex_t media_thumb_mutex;
static void media_thumb_mutex_init(void)
{
	static bool is_first = true;
	if (is_first == true)
	{
		is_first = false;
		ak_thread_mutex_init(&media_thumb_mutex, NULL);
	}
}

bool media_thumb_decode_clear(int handle)
{
	if (handle == -1)
	{
		return false;
	}

	struct ak_vdec_frame frame = {0};
	while (ak_vdec_get_frame(handle, &frame) == 0)
	{
		ak_vdec_release_frame(handle, &frame);
	}

	ak_vdec_end_stream(handle);
	while (1)
	{
		int status = 0;
		if (ak_vdec_get_frame(handle, &frame) == 0)
		{
			ak_vdec_release_frame(handle, &frame);
		}
		ak_vdec_get_decode_finish(handle, &status);
		ak_sleep_ms(100);
		if (status)
		{
			break;
		}
		// ak_sleep_ms(100);
	}
	ak_vdec_clear_buff(handle);
	return true;
}

bool media_thumb_decode_refresh(int handle)
{
	if (handle == -1)
	{
		return false;
	}

	printf("%s ===========================>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n",__func__);
	struct ak_vdec_frame frame = {0};
	ak_vdec_end_stream(handle);
	while (1)
	{
		int status = 0;
		if (ak_vdec_get_frame(handle, &frame) == 0)
		{
			ak_vdec_release_frame(handle, &frame);
		}
		ak_vdec_get_decode_finish(handle, &status);
		if (status)
		{
			break;
		}
		ak_sleep_ms(1);
	}
	ak_vdec_clear_buff(handle);
	return true;
}
static void media_thumb_decode_close(int handle)
{
	if (handle == -1)
	{
		return;
	}

	media_thumb_decode_clear(handle);
#if 0
	struct ak_vdec_frame frame = {0};
	ak_vdec_end_stream(handle);
	while(1)
	{
		int status = 0;
		if(ak_vdec_get_frame(handle, &frame) == 0)
		{	
			ak_vdec_release_frame(handle, &frame);
		}	
		else if(ak_vdec_get_decode_finish(handle, &status) ||(status))
		{
				break;
		}
		ak_sleep_ms(1);
	}
#endif
	ak_vdec_close(handle);
}

static void *media_thumb_open_task(void *arg)
{
	printf("media_thumb_open_task====================++>S\n");
	media_thumb_thread = true;

	if (media_mjpeg_handle_id == -1)
	{
		struct ak_vdec_param param;
		memset(&param, 0, sizeof(struct ak_vdec_param));
		param.vdec_type = MJPEG_ENC_TYPE;
		param.output_type = AK_YUV420SP;
		param.sc_width = media_thumb_width;
		param.sc_height = media_thumb_height;
		param.frame_buf_num = 2;
		param.stream_buf_size = 307200;
		if (ak_vdec_open(&param, &media_mjpeg_handle_id))
		{
			printf("open video mjpeg device fail \n\r");
		}
		ak_vdec_clear_buff(media_mjpeg_handle_id);
	}

	if (media_h264_handle_id == -1)
	{
		struct ak_vdec_param param;
		memset(&param, 0, sizeof(struct ak_vdec_param));
		param.vdec_type = H264_ENC_TYPE;
		param.output_type = AK_YUV420SP;
		param.sc_width = media_thumb_width;
		param.sc_height = media_thumb_height;
		param.frame_buf_num = 3;
		param.stream_buf_size = 4*1024*1024;
		if (ak_vdec_open(&param, &media_h264_handle_id))
		{
			printf("open video h264 device fail \n\r");
		}
		ak_vdec_clear_buff(media_h264_handle_id);
	}
	media_decode_device_ready = true;
	while (madia_thumb_task_run == true)
	{
		ak_sleep_ms(10);
	}

	/*
	 *
	 *	增加互斥锁，防止在主线程加载解码图片的时候将解码器关闭
	 */
	ak_thread_mutex_lock(&media_thumb_mutex);
	media_thumb_decode_close(media_mjpeg_handle_id);
	media_thumb_decode_close(media_h264_handle_id);
	media_decode_device_ready = false;
	ak_thread_mutex_unlock(&media_thumb_mutex);

	media_mjpeg_handle_id = -1;
	media_h264_handle_id = -1;

	media_thumb_thread = false;
	printf("===========<<< media thumb close finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

bool media_thumb_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (media_thumb_thread == false)
		{
			return true;
		}
		usleep(10 * 1000);
	}
	printf("%s=======>%d\n",__func__,false);
	return false;
}

bool media_thumb_device_open(int width, int height)
{
	media_thumb_mutex_init();

	if (madia_thumb_task_run == true)
	{
		return false;
	}

	if (media_thumb_wait_thread_quit() == false)
	{
		printf("thumb thread wait error \n");
		return false;
	}

	media_thumb_width = width;
	media_thumb_height = height;

	media_decode_device_ready = false;
	madia_thumb_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, media_thumb_open_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

int media_h264_decode_handle_id_get(void)
{
	return media_h264_handle_id;
}

int media_mjpeg_decode_handle_id_get(void)
{
	return media_mjpeg_handle_id;
}

bool media_thumb_device_close(void)
{
	printf("%s================================>\n",__func__);
	if (madia_thumb_task_run == false)
	{
		return false;
	}

	madia_thumb_task_run = false;
	return true;
}

static bool jpeg_data_get(const char *file_path, unsigned char **buffer, int *size)
{
	struct stat statbuf;
	stat(file_path, &statbuf);
	if (statbuf.st_size < 1)
	{
		return false;
	}

	int fd = open(file_path, O_RDONLY);
	if (fd < 0)
	{
		return false;
	}

	*size = statbuf.st_size;
	*buffer = (unsigned char *)ak_mem_alloc(MODULE_ID_VDEC, *size);
	read(fd, *buffer, *size);
	close(fd);
	return true;
}

void thumb_stream_send(int handle, unsigned char *pdata, int len)
{
	int dec_len = 0;
	int read_len = len;
	int send_len = 0;
	while (read_len > 0)
	{
		ak_vdec_send_stream(handle, &pdata[send_len], read_len, NONBLOCK, &dec_len);
		read_len -= dec_len;
		send_len += dec_len;

	}
}

static void thumb_frame_copy_to_bg_sysetm(struct ak_vdec_frame *frame, int x, int y, int w, int h)
{
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = frame->frame_obj.data.pitch_width;
	src.height = frame->frame_obj.data.pitch_height;
	src.pos_left = 0;
	src.pos_top = 0;
	src.pos_width = frame->width;
	src.pos_height = frame->height;
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.width = LV_HOR_RES_MAX;
	dst.height = LV_VER_RES_MAX;
	dst.pos_left = x;
	dst.pos_top = y;
	dst.pos_width = w;
	dst.pos_height = h;
	extern unsigned long system_bg_phyaddres_get(void);
	dst.phyaddr = system_bg_phyaddres_get();
	ak_tde_opt_scale(&src, &dst);
}
static bool thumb_frame_display(int handle, int x, int y, int w, int h)
{
	struct ak_vdec_frame frame = {0};
	if ((ak_vdec_get_frame(handle, &frame)) == 0)
	{
		thumb_frame_copy_to_bg_sysetm(&frame, x, y, w, h);
		ak_vdec_release_frame(handle, &frame);
		return true;
	}
	return false;
}
static bool jpeg_thumb_load(int x, int y, int w, int h, const char *file_path)
{
	unsigned char *jpg_buffer = NULL;
	int jpg_size = 0;
	if (jpeg_data_get(file_path, &jpg_buffer, &jpg_size) == false)
	{
		return false;
	}

	int timeout = 1000;

	bool is_first = true;
	while (timeout--)
	{
		if (media_decode_device_ready == false)
		{
			ak_sleep_ms(1);
			continue;
		}

		if (is_first == true)
		{
			media_thumb_decode_clear(media_mjpeg_handle_id);
			is_first = false;
		}

		thumb_stream_send(media_mjpeg_handle_id, jpg_buffer, jpg_size);
		if (thumb_frame_display(media_mjpeg_handle_id, x, y, w, h) == true)
		{
			break;
		}
		else
		{
			ak_mem_free(jpg_buffer);
			return false;
		}
		ak_sleep_ms(1);
	}

	ak_mem_free(jpg_buffer);

	return true;
}

static bool video_thumb_wait_ready(void)
{
	int timeout = 1000;
	while (timeout--)
	{
		if (media_decode_device_ready == true)
		{
			return true;
		}
		ak_sleep_ms(1);
	}
	return false;
}
static bool video_thumb_load(int x, int y, int w, int h, const char *file_path)
{
	printf("%s ======================***********************************\n",__func__);
	avi_t *avi_file = AVI_open_input_file(file_path, 1);
	if (avi_file == NULL)
	{
		printf("%s %d  %s\n", __func__, __LINE__, file_path);
		return false;
	}

	int *handle_id = NULL;
	char *format = AVI_video_compressor(avi_file);
	if (strcmp(format, "H264") == 0)
	{
		handle_id = &media_h264_handle_id;
	}
	else if (strcmp(format, "MJPG") == 0)
	{
		handle_id = &media_mjpeg_handle_id;
	}

	int iskeyframe;
	int total = AVI_video_frames(avi_file);

	char *frame_buffer = NULL;
	int i = 0;
	bool is_first = true;

	for (i = 0; i < total; i++)
	{
		if (AVI_set_video_position(avi_file, i) != 0)
		{
			break;
		}

		int frame_size = AVI_frame_size(avi_file, i);

		if (frame_size <= 0)
		{
			continue;
		}
		frame_buffer = (char *)ak_mem_alloc(MODULE_ID_VDEC, frame_size);
		if (frame_buffer == NULL)
		{
			break;
		}

		if (AVI_read_frame(avi_file, frame_buffer, &iskeyframe) < 0)
		{
			break;
		}

		if (video_thumb_wait_ready() == false)
		{
			break;
		}

		if (is_first == true)
		{
			media_thumb_decode_clear(*handle_id);
			is_first = false;
		}
		thumb_stream_send(*handle_id, (unsigned char *)frame_buffer, frame_size);
		ak_mem_free(frame_buffer);
		frame_buffer = NULL;
		if (thumb_frame_display(*handle_id, x, y, w, h) == true)
		{
			break;
		}
	}

	if (frame_buffer != NULL)
	{
		ak_mem_free(frame_buffer);
	}
	AVI_close(avi_file);
	printf("%s ======================***********************************\n",__func__);
	return true;
}
bool media_thumb_load(int x, int y, int w, int h, const char *file_path)
{
	char *p = strrchr(file_path, '.');
	if (p == NULL)
	{
		return false;
	}

	int str_len = strlen(p);
	if (str_len != 4)
	{
		return false;
	}

	if (video_thumb_wait_ready() == false)
	{
		return false;
	}

	bool reslut = true;
	ak_thread_mutex_lock(&media_thumb_mutex);
	if (media_decode_device_ready == false)
	{
		ak_thread_mutex_unlock(&media_thumb_mutex);
		return reslut;
	}

	if ((p[1] == 'j') || (p[1] == 'J'))
	{
		reslut = jpeg_thumb_load(x, y, w, h, file_path);
	}
	else if ((p[1] == 'A') || (p[1] == 'a'))
	{
		reslut = video_thumb_load(x, y, w, h, file_path);
	}
	ak_thread_mutex_unlock(&media_thumb_mutex);
	return reslut;
}
