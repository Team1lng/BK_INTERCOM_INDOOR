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
#include "../../include/tuya/tuya_ipc_api.h"

#include "ak_tde.h"

#define JPEG_WIDTH 1024
#define JPEG_HEIGHT 600

extern bool video_auto_record_falg;

static bool jpeg_record_task_run = false;

typedef struct
{
	char file_path[128];
	char recode_mode;
	bool has_audio;
	int width;
	int height;
	bool frame_type; // 0:h264,1:mjpeg 2:h265
} record_info;

#if 0
static bool jpeg_get_raw_video_data(unsigned char *addres, int width, int height)
{
	return true;
	int ms = 100;
	extern unsigned char *video_raw_lcd_get(unsigned long long *timestamp);
	while (video_raw_lcd_get(NULL) == NULL)
	{
		ak_sleep_ms(1);
		ms--;
		if (ms == 0)
		{
			return false;
		}
	}
	return true;
}

#else
static bool jpeg_get_raw_video_data(unsigned char *addres, int width, int height)
{
   int ms = 100;
   extern unsigned char *video_raw_lcd_get(unsigned long long *timestamp);
   unsigned char *frame_addr = NULL;
   while ((frame_addr = video_raw_lcd_get(NULL)) == NULL)
   {
      ak_sleep_ms(1);
      ms--;
      if (ms == 0)
      {
         return false;
      }
   }

   struct ak_tde_layer dst, src;
   src.format_param = GP_FORMAT_YUV420SP;
   src.width = width;// src_width;
   src.height = height; // src_height;
   src.pos_left = 0;
   src.pos_top = 0;
   src.pos_width = width;
   src.pos_height = height;
   ak_mem_dma_vaddr2paddr(video_raw_lcd_get(NULL), (unsigned long *)&src.phyaddr);
   

   dst.format_param = GP_FORMAT_YUV420SP;
   dst.width = width;
   dst.height = height;
   dst.pos_left = 0;
   dst.pos_top = 0;
   dst.pos_width = width;
   dst.pos_height = height;
   ak_mem_dma_vaddr2paddr(addres, (unsigned long *)&dst.phyaddr);
   ak_tde_opt_blit(&src, &dst);

   //memcpy(addres, frame_addr, width * height * 3 / 2);
   return true;
}
#endif
static int jpeg_encode_open(int width, int heigh)
{
	struct venc_param ve_param;
	ve_param.width = width;
	ve_param.height = heigh;
	ve_param.fps = 25;					 // fps set
	ve_param.goplen = 50;					 // gop set
	ve_param.target_kbps = 800;				 // 800;		 //k bps
	ve_param.max_kbps = 1024;				 // 1024;		 //max kbps
	ve_param.br_mode = BR_MODE_VBR;				 // BR_MODE_VBR;//BR_MODE_CBR; //br mode
	ve_param.minqp = 25;					 // qp set
	ve_param.maxqp = 50;					 // qp max value
	ve_param.initqp = (ve_param.minqp + ve_param.maxqp) / 2; // qp value
	ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;		 // jpeg qlevel
	ve_param.chroma_mode = CHROMA_4_2_0;			 // chroma mode
	ve_param.max_picture_size = 0;				 // 0 means default
	ve_param.enc_level = 50;				 // 50;				 //enc level
	ve_param.smart_mode = 0;				 // smart mode set
	ve_param.smart_goplen = 100;				 // smart mode value
	ve_param.smart_quality = 50;				 // quality
	ve_param.smart_static_value = 0;			 // value
	ve_param.enc_out_type = MJPEG_ENC_TYPE;			 // enc type
	ve_param.profile = PROFILE_JPEG;

	int jpeg_handle_id = -1;
	ak_venc_open(&ve_param, &jpeg_handle_id);

	return jpeg_handle_id;
}

static bool jpeg_stream_write_file(unsigned char *data, int size, const char *file_path)
{
	int fd = open(file_path, O_WRONLY | O_CREAT);
	if (fd < 0)
	{
		return false;
	}

	write(fd, data, size);

	close(fd);

	return true;
}

static bool jpeg_encode_write_frame(int handle_id, unsigned char *addres, int frame_size, const char *file_path, struct video_stream *stream)
{
	bool reslut = false;
	// struct video_stream stream = {0};
	printf("[%s:%d] file:%s \n", __func__, __LINE__, file_path);
	if (ak_venc_encode_frame(handle_id, addres, frame_size, NULL, stream) == 0)
	{
		printf("[%s:%d] file:%s \n", __func__, __LINE__, file_path);
		reslut = jpeg_stream_write_file(stream->data, stream->len, file_path);
	}
	else
	{
		printf("[%s:%d] video encode frame mjpeg fail :%s \n", __func__, __LINE__, file_path);
	}
	printf("[%s:%d] file:%s \n", __func__, __LINE__, file_path);
	return reslut;
}

static int push_jpg_to_tuya_type = 0;

void jpg_push_to_tuya(int type)
{

	printf("jpg_push_to_tuya ======================>>>\n\r");
	push_jpg_to_tuya_type = type;
}

extern bool is_online_tuya_cloud(void);
#include"leo_api.h"
static void *jpeg_record_task(void *arg)
{
	record_info *info = (record_info *)arg;


	int count = 300;
	extern bool get_video_data_display_state(void);
	while(!(count--) && !get_video_data_display_state())
	{
		ak_sleep_ms(1);
	};

	int frame_size = JPEG_WIDTH * JPEG_HEIGHT * 3 / 2;
	unsigned char *addres = video_raw_lcd_get(NULL);
	// ak_mem_dma_alloc(MODULE_ID_VENC, frame_size);
	extern IPC_REGISTER_STATUS tuya_ipc_register_status_get(void);
	bool reslut = false;
	do
	{
		if (addres == NULL)
		{
			if (jpeg_get_raw_video_data(addres, JPEG_WIDTH, JPEG_HEIGHT) == false)
			{
				printf("[%s:%d] get raw video fail \n", __func__, __LINE__);
				break;
			}
		}
		int hande_id = jpeg_encode_open(JPEG_WIDTH, JPEG_HEIGHT);
		if (hande_id == -1)
		{
			printf("[%s:%d] encode frame open fail \n", __func__, __LINE__);
			break;
		}
		
		struct video_stream stream;
		if (jpeg_encode_write_frame(hande_id, addres, frame_size, info->file_path, &stream) == false)
		{
			printf("[%s:%d] encode frame open fail \n", __func__, __LINE__);
			break;
		}

		if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED && push_jpg_to_tuya_type != 0)
		{
			if (is_online_tuya_cloud() == true)
			{
				push_jpg_to_tuya_type = 0;
				// struct video_stream stream;
				// if (ak_venc_encode_frame(hande_id, addres, frame_size, NULL, &stream) == 0)
				{
					if(info->recode_mode != REC_MODE_MOTION)
						tuya_ipc_notify_door_bell_press((char *)stream.data, stream.len, NOTIFICATION_CONTENT_JPEG);
					else
						tuya_ipc_notify_motion_detect((char *)stream.data, stream.len, NOTIFICATION_CONTENT_JPEG);
				}
			}
		}
		
		ak_venc_release_stream(hande_id,&stream);
		ak_venc_close(hande_id);
		reslut = true;
	} while (0);

	if (addres != NULL)
	{
		; // ak_mem_dma_free(addres);
	}
	system("sync");
	printf("\n jpeg %s record %s \n", info->file_path, reslut ? "success" : "fail");
	extern bool record_jpeg_event_push(char record_mode);
	record_jpeg_event_push(info->recode_mode);

	extern void detect_sd_free_space(void);
	detect_sd_free_space();

	push_jpg_to_tuya_type = 0;
	jpeg_record_task_run = false;
	ak_thread_exit();
	return NULL;
}

static record_info record_info1 = {{0}, 0};
bool jpg_record(const char *file_path, char record_mode)
{
	if (jpeg_record_task_run == true)
	{
		printf("jpeg record thread runing \n");
		return false;
	}

	jpeg_record_task_run = true;

	strcpy(record_info1.file_path, file_path);
	record_info1.recode_mode = record_mode;

	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, jpeg_record_task, &record_info1, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool close_jpg_record(void)
{
	if (jpeg_record_task_run == false)
	{
		return false;
	}

	jpeg_record_task_run = false;
	return true;
}

bool is_jpg_record_ing(void)
{
	return jpeg_record_task_run;
}


static bool sent_tuya_task_run = false;
bool is_send_tuya_jgp(void)
{
	return sent_tuya_task_run;
}

static void *sent_tuya_record_task(void *arg)
{
	char*mode = (char*)arg;

	int frame_size = JPEG_WIDTH * JPEG_HEIGHT * 3 / 2;
	int count = 300;


	extern bool get_video_data_display_state(void);
	while(!(count--) && !get_video_data_display_state())
	{
		ak_sleep_ms(1);
	};

	printf("@@@@@@@@@@@@@@@@@@@>>>>>>>>>>>>>>:%d %d:\n", get_video_data_display_state(), *mode);
	unsigned char *addres = video_raw_lcd_get(NULL); // ak_mem_dma_alloc(MODULE_ID_VENC, frame_size);

	// bool reslut = false;
	do
	{
		// if((video_auto_record_falg == false))
		// 	break;
		if (addres == NULL)
		{
			if (jpeg_get_raw_video_data(addres, JPEG_WIDTH, JPEG_HEIGHT) == false)
			{
				printf("[%s:%d] get raw video fail \n", __func__, __LINE__);
				break;
			}
		}

		int hande_id = jpeg_encode_open(JPEG_WIDTH, JPEG_HEIGHT);
		if (hande_id == -1)
		{
			printf("[%s:%d] encode frame open fail \n", __func__, __LINE__);
			break;
		}

		// if(jpeg_encode_write_frame(hande_id,addres,frame_size,file_path) == false)
		// {
		// 	printf("[%s:%d] encode frame open fail \n",__func__,__LINE__);
		// 	break;
		// }

		struct video_stream stream;
		int a = ak_venc_encode_frame(hande_id, addres, frame_size, NULL, &stream);
		if (a == 0)
		{
			printf("@@@@@@@@@@@@@@@@@@@>>>>>>>>>>>>>>:%s %d:\n", __func__,__LINE__);
			if(*mode != REC_MODE_MOTION)
				tuya_ipc_notify_door_bell_press((char *)stream.data, stream.len, NOTIFICATION_CONTENT_JPEG);
			else
				tuya_ipc_notify_motion_detect((char *)stream.data, stream.len, NOTIFICATION_CONTENT_JPEG);
		}

		ak_venc_release_stream(hande_id,&stream);
		ak_venc_close(hande_id);

		// reslut = true;

	} while (0);

	if (addres != NULL)
	{
		; // ak_mem_dma_free(addres);
	}

	// printf("\n jpeg %s record %s \n",file_path,reslut?"success":"fail");
	// extern bool record_jpeg_event_push(char record_mode);
	// record_jpeg_event_push(*file_path);
	printf("@@@@@@@@@@@@@@@@@@@>>>>>>>>>>>>>>:%s %d:\n", __func__, __LINE__);
	sent_tuya_task_run = false;
	ak_thread_exit();
	return NULL;
}

static char tuya_record_mode;
bool send_tuya_record(char record_mode)
{
	if (sent_tuya_task_run == true)
	{
		Debug_Lib("sent tuya record thread runing \n");
		return false;
	}

	Debug_Lib("%s======================>>%d\n",__func__,__LINE__);
	sent_tuya_task_run = true;
	tuya_record_mode = record_mode;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, sent_tuya_record_task, &tuya_record_mode, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}
