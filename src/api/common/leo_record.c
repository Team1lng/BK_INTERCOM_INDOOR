#include "leo_api.h"
#include "file_api.h"
#include "stdio.h"
#include "video_decode.h"
/***********************************

检查记录文件参数是否正确

************************************/
static bool check_record_video_parameter(char mode, char audio_from, char video_channel)
{
	if (is_video_recording() == true)
	{

		// printf("Video recording is working \n\r");
		return false;
	}

	return true;
}

/***********************************

打开记录音频通道。
return
	true:记录声音，false:不记录声音

************************************/
/* static bool record_audio_channel_open(char audio_from,MONITOR_CH video_channel)
{
	if(audio_from)
	{
		return true;
	}
	return false;
} */
/*
 *  0:h264,1:mjpeg
 */
static char record_video_info(MONITOR_CH ch, int *width, int *height)
{
	if ((ch == MON_CH_DOOR_1) || (ch == MON_CH_DOOR_2))
	{
		*width = DECODE_WIDTH;
		*height = DECODE_HIGHT;
		return 0;
	}
	else if ((ch == MON_CH_CCTV_1) || (ch == MON_CH_CCTV_2))
	{
		*width = 1280;
		*height = 720;
		// extern void live555SendSPSandPPS(void);
		// live555SendSPSandPPS();
		return 0;
	}
	return 1;
}

static enum delete_flag check_record_type_parameter(char mode)
{
	switch (mode)
	{
	case REC_MODE_MANUAL:
	case REC_MODE_AUTO:
		return FILE_TYPE_SD_CALL;
	case REC_MODE_MESSAGE:
		return FILE_TYPE_SD_MSG;
	case REC_MODE_MOTION:
		return FILE_TYPE_SD_MOTION;
	case REC_MODE_ALARM:
		return FILE_TYPE_SD_ALARM;
	default:
		return FILE_TYPE_SD_MIXED;
	}

	return true;
}

void alarm_record_add(int sense)
{
	if (is_sdcard_insert() == false)
	{
		return;
	}

	char file_path[128] = {0};
	if (create_one_media_file(FILE_TYPE_SD_ALARM, sense, REC_MODE_ALARM, TEXT_TYPE, file_path) == false)
	{

		printf("failed to create media file \n\r");
		return;
	}
	system("sync");
}

MONITOR_CH video_channel_temp = MON_CH_NONE;
char mode_temp = 0;
char record_video_type(void)
{
	return mode_temp;
}
static void record_video_finish_callback(const char *path)
{
	if (is_sdcard_insert() == false)
	{
		mode_temp = 0;
		return;
	}

	char file_path[128] = {0};
	if (create_one_media_file(check_record_type_parameter(mode_temp), video_channel_temp, mode_temp, VIDEO_TYPE, file_path) == false)
	{
		mode_temp = 0;
		printf("failed to create media file \n\r");
		return;
	}

	int ret = rename(path, file_path);
	printf("TEMP_FILE:%s   FILE_PATH:%s       RENAME RET:%d\n\r", path, file_path, ret);
	// char cmd[128] = {0};
	// sprintf(cmd, "mv %s %s", path, file_path);
	// system(cmd);
	// media_file_bad_check(file_path);
	system("sync");

	usleep(1000);

	if (access(file_path, F_OK) != 0)
	{
		printf("create_one_media_file %s not exist  !!! ====>>> delete .config info\n\r", file_path);
		int total = media_file_total_get(check_record_type_parameter(mode_temp), false);
		media_file_delete(check_record_type_parameter(mode_temp), total - 1);
	}
	mode_temp = 0;
	// printf("record video:%s \n", file_path);
}
/**********************************************
mode: 记录模式 手动,自动，移动侦测。
audio_frome: 记录音频来源。
	0x00:不记录音频。
	0x01:记录户外机
	0x02:记录室内机
	0x03:记录室内机和户外机
**********************************************/
bool record_video_start(char mode, char audio_from, MONITOR_CH video_channel)
{

	if (is_jpg_record_ing())
	{
		Debug_Lib("\n\n\n\n");
		return false;
	}

	if (check_record_video_parameter(mode, audio_from, video_channel) == false)
	{
		Debug_Lib("\n\n\n\n");
		return false;
	}
	if (video_channel == MON_CH_CCTV_1 || video_channel == MON_CH_CCTV_2)
	{
		audio_from = false;
	}

	Debug_Lib("\n\n\n\n");
	int width = 0, height = 0;
	char video_type = record_video_info(video_channel, &width, &height);
#define VIDEO_TEMP_FILE_PATH "/mnt/tf/video.AVI"

	if (video_channel == MON_CH_DOOR_1 && network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()), 0) == curr_network_video_receive_eth_id_get())
	{
		request_send_I_frame_cmd(DEVICE_OUTDOOR_1);
	}
	else if (video_channel == MON_CH_DOOR_2 && network_common_socket_eth_p_get(0, network_get_id_outdoor2(network_local_device_get()), 0) == curr_network_video_receive_eth_id_get())
	{
		request_send_I_frame_cmd(DEVICE_OUTDOOR_2);
	}

	if (video_record_start(VIDEO_TEMP_FILE_PATH, audio_from, width, height, video_type, mode, check_record_type_parameter(mode), record_video_finish_callback) == true)
	{
		video_channel_temp = video_channel;
		mode_temp = mode;
		printf("open mux video succee %s \n\r", VIDEO_TEMP_FILE_PATH);
		return true;
	}
	return false;
}

bool check_record_video_stop_parameter(char audio_flag)
{

	if (is_video_recording() == false)
	{
		// printf("Video recording is working \n\r");
		return false;
	}

	return true;
}

/***********************************

关闭视频记录
	audio_flag: 关闭记录音频。
		0x01: 关闭所有音频
		0x02: 只是关闭进入mic的通道
************************************/
bool record_video_stop(char audio_flag)
{

	if (check_record_video_stop_parameter(audio_flag) == false)
	{

		return false;
	}

	video_record_stop();
	return true;
}

static bool check_record_picture_parameter(void)
{
	if (is_jpg_record_ing() == true)
	{
		return false;
	}

	return true;
}

static bool record_pictrue_file_path_get(char mode, MONITOR_CH video_channel, char *file_path)
{

	if (create_one_media_file(/* is_sdcard_insert()?  */ check_record_type_parameter(mode) /* :FILE_TYPE_FLASH_PHOTO */, video_channel, mode, PHOTO_TYPE, file_path) == false)
	{
		printf("Error getting file path \n\r");
		return false;
	}
	return true;
}

int flash_free_space(void)
{
	FILE *fp;
	int use;
	char buffer[80];
	char com[64];
	sprintf(com, "df  | grep '%s'| awk '{print $(NF-1)}' | awk -F'%%' '{print $1}'", APP_DEFAULT_PATH);
	fp = popen(com, "r");
	fgets(buffer, sizeof(buffer), fp);
	use = atoi(buffer);
	pclose(fp);
	return use;
}

bool record_pictrue_start(char mode, MONITOR_CH video_channel)
{
	bool is_video_recording(void);
	if (is_video_recording())
	{
		return false;
	}

	if (check_record_picture_parameter() == false)
	{
		return false;
	}

	char file_path[64] = {0};
	if (record_pictrue_file_path_get(mode, video_channel, file_path) == false)
	{

		Debug_Lib("failed to create media file \n\r");
		return false;
	}
	Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
	return jpg_record(file_path, mode);
}
