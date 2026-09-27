#include "leo_api.h"
#include "stdbool.h"
#include "video_decode.h"
#include "file_api.h"
#include "network_common.h"
#include "../audio/audio_input.h"
#include "../audio/audio_decode.h"
#include "stdio.h"

static MONITOR_ENTER_WAY monitor_enter_flag = MONITOR_ENTER_NONE;

extern void fb_video_mode_enable(bool);

void monitor_enter_way_set(MONITOR_ENTER_WAY flag)
{
	monitor_enter_flag = flag;
}

MONITOR_ENTER_WAY monitor_enter_way_get(void)
{
	return monitor_enter_flag;
}

static MONITOR_CH monitor_channel = MON_CH_NONE;

void monitor_reset(bool close_decode)
{
	// extern bool rtsp_stream_close();
	// rtsp_stream_close();
	// extern unsigned long long os_get_ms(void);
	// unsigned long long x = os_get_ms();
	if (close_decode)
		video_decode_close();

	// Debug_Lib("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();

	network_video_receive_package_close();

	network_video_send_package_close();

	if (!close_decode)
		video_decode_queue_reset();

	extern bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h);
	system_bg_fill_color(0x00, 0, 0, 1024, 600);
}

extern bool rtsp_stream_open(char *url);

void monitor_open(bool reset)
{
	Debug_Lib("[%s]==========================>>>\n", __func__);
	if (monitor_channel == MON_CH_NONE)
	{
		return;
	}
	if (reset)
		monitor_reset(true); // 22ms

	extern bool rtsp_stream_close(void);
	rtsp_stream_close();
	video_decode_queue_reset();
#ifndef DHCP_IPCAMERA
	static char buf[96] = {0};
#endif

	if (monitor_channel == MON_CH_DOOR_1 && (device_online_state_get(DEVICE_OUTDOOR_1) || monitor_enter_way_get() == MONITOR_ENTER_CALL))
	{
		int eth_p_id = network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()), 0);
		video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);
		network_video_receive_package_open(eth_p_id);	  // while(1);
	}
	else if (monitor_channel == MON_CH_DOOR_2 && (device_online_state_get(DEVICE_OUTDOOR_2) || monitor_enter_way_get() == MONITOR_ENTER_CALL))
	{
		int eth_p_id = network_common_socket_eth_p_get(0, network_get_id_outdoor2(network_local_device_get()), 0);
		video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);
		network_video_receive_package_open(eth_p_id);
	}
	else if (monitor_channel == MON_CH_CCTV_1 && monitor_config_get()->cctv1 != NULL)
	{
		video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);

#ifndef DHCP_IPCAMERA
		if (monitor_config_get()->cctv1->model)
		{ // HIK
			sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1", monitor_config_get()->cctv1->account, monitor_config_get()->cctv1->pwd, monitor_config_get()->cctv1->ip);
		}
		else
		{ // DAH
			sprintf(buf, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", monitor_config_get()->cctv1->account, monitor_config_get()->cctv1->pwd, monitor_config_get()->cctv1->ip);
		}
		Debug_Lib("==>>%s\n\n\n\n\n", buf);
		rtsp_stream_open(buf);
#else
		if (monitor_config_get()->cctv1->url[0] == 'r')
			rtsp_stream_open(monitor_config_get()->cctv1->url);
#endif
	}
	else if (monitor_channel == MON_CH_CCTV_2 && monitor_config_get()->cctv2 != NULL)
	{
		video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);

#ifndef DHCP_IPCAMERA
		if (monitor_config_get()->cctv2->model)
		{ // HIK
			sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1", monitor_config_get()->cctv2->account, monitor_config_get()->cctv2->pwd, monitor_config_get()->cctv2->ip);
		}
		else
		{ // DAH
			sprintf(buf, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", monitor_config_get()->cctv2->account, monitor_config_get()->cctv2->pwd, monitor_config_get()->cctv2->ip);
		}
		Debug_Lib("==>>%s\n\n\n\n\n", buf);
		rtsp_stream_open(buf);
#else
		if (monitor_config_get()->cctv2->url[0] == 'r')
			rtsp_stream_open(monitor_config_get()->cctv2->url);
#endif
	}
	else
	{
	}

	if (monitor_enter_way_get() != MONITOR_ENTER_TUYA)
	{
		if (monitor_channel == MON_CH_DOOR_1 || monitor_channel == MON_CH_DOOR_2)
			fb_video_mode_enable(device_online_state_get(monitor_channel + DEVICE_INDOOR_ID6));
		else
			fb_video_mode_enable(true);
	}
	else
	{
		extern bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h);
		system_bg_fill_color(0x00, 0, 0, 1024, 600);
	}
}
extern bool video_decode_pause(bool keep);
void monitor_switch(void)
{
	// if(monitor_channel != MON_CH_DOOR_1 &&
	// 	monitor_channel != MON_CH_DOOR_2)
	// {
	// 	return ;
	// }
	int curr_eth_p_id = curr_network_video_receive_eth_id_get();
	int slave_id = 0;

	switch (monitor_channel)
	{
	case MON_CH_DOOR_1:
		slave_id = network_get_id_outdoor1(network_local_device_get());
		break;
	case MON_CH_DOOR_2:
		slave_id = network_get_id_outdoor2(network_local_device_get());
		break;
	case MON_CH_CCTV_1:
		slave_id = 0;
		break;
	case MON_CH_CCTV_2:
		slave_id = 0;
		break;

	default:
		break;
	}
	int eth_p_id = network_common_socket_eth_p_get(0, slave_id, 0);
	if (monitor_channel == MON_CH_NONE)
	{
		return;
	}

	Debug_Lib("%s==========================>>>0x%x   0x%x\n", __func__, curr_eth_p_id, eth_p_id);
	if (curr_eth_p_id != eth_p_id)
		network_video_receive_package_close();

	// video_decode_queue_reset();

	extern bool rtsp_stream_close(void);
	rtsp_stream_close();
	// video_raw_clear();

	if (get_video_decode_state() == true && curr_eth_p_id != eth_p_id)
		video_decode_pause(true);

#ifndef DHCP_IPCAMERA
	static char buf[96] = {0};
#endif

	extern bool rtsp_stream_open(char *url);

	// if (!video_decode_resolution_contrast(DECODE_WIDTH, DECODE_HIGHT))
	{
		video_decode_close();
	}

	if (get_video_decode_state() == false && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
	{
		video_decode_open(0, DECODE_WIDTH, DECODE_HIGHT); // 640, 360);
	}
	// ak_sleep_ms(3000);
	if (monitor_channel == MON_CH_DOOR_1 && (device_online_state_get(DEVICE_OUTDOOR_1) || monitor_enter_way_get() == MONITOR_ENTER_CALL))
	{

		int eth_p_id = network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()), 0);
		network_video_receive_package_open(eth_p_id); // while(1);
	}
	else if (monitor_channel == MON_CH_DOOR_2 && (device_online_state_get(DEVICE_OUTDOOR_2) || monitor_enter_way_get() == MONITOR_ENTER_CALL))
	{

		int eth_p_id = network_common_socket_eth_p_get(0, network_get_id_outdoor2(network_local_device_get()), 0);
		network_video_receive_package_open(eth_p_id);
	}
	else if (monitor_channel == MON_CH_CCTV_1 && monitor_config_get()->cctv1 != NULL)
	{

#ifndef DHCP_IPCAMERA
		if (monitor_config_get()->cctv1->model)
		{ // HIK
			sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1", monitor_config_get()->cctv1->account, monitor_config_get()->cctv1->pwd, monitor_config_get()->cctv1->ip);

			// sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1","admin","Bcom12345", "192.168.170.103");
		}
		else
		{ // DAH
			sprintf(buf, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", monitor_config_get()->cctv1->account, monitor_config_get()->cctv1->pwd, monitor_config_get()->cctv1->ip);
		}
		Debug_Lib("==>>%s\n\n\n\n\n", buf);
		rtsp_stream_open(buf);
#else
		if (monitor_config_get()->cctv1->url[0] == 'r')
		{
			rtsp_stream_open(monitor_config_get()->cctv1->url);

			// rtsp_stream_open("rtsp://admin:123456@172.16.0.117:8554/video");
		}
#endif
	}
	else if (monitor_channel == MON_CH_CCTV_2 && monitor_config_get()->cctv2 != NULL)
	{

#ifndef DHCP_IPCAMERA
		if (monitor_config_get()->cctv2->model)
		{ // HIK
			sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1", monitor_config_get()->cctv2->account, monitor_config_get()->cctv2->pwd, monitor_config_get()->cctv2->ip);
			// sprintf(buf, "rtsp://%s:%s@%s:554/Streaming/Channels/1","admin","Bcom12345", "192.168.170.103");
		}
		else
		{ // DAH
			sprintf(buf, "rtsp://%s:%s@%s:554/cam/realmonitor?channel=1&subtype=1", monitor_config_get()->cctv2->account, monitor_config_get()->cctv2->pwd, monitor_config_get()->cctv2->ip);
		}
		Debug_Lib("==>>%s\n\n\n\n\n", buf);
		rtsp_stream_open(buf);
#else
		if (monitor_config_get()->cctv2->url[0] == 'r')
			rtsp_stream_open(monitor_config_get()->cctv2->url);
#endif
	}

	extern bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h);
	system_bg_fill_color(0x00, 0, 0, 1024, 600);
	if (monitor_enter_way_get() != MONITOR_ENTER_TUYA)
	{
		if (monitor_channel == MON_CH_DOOR_1 || monitor_channel == MON_CH_DOOR_2)
			fb_video_mode_enable(device_online_state_get(monitor_channel + DEVICE_INDOOR_ID6));
		else
			fb_video_mode_enable(true);
	}
	else
	{
	}
}

void monitor_close(void)
{
	monitor_reset(true);

	monitor_channel = MON_CH_NONE;
	extern bool rtsp_stream_close(void);
	rtsp_stream_close();
}

void monitor_close_1(void)
{
	monitor_reset(true);
	extern bool rtsp_stream_close(void);
	rtsp_stream_close();
	// video_raw_clear();
}

MONITOR_CH monitor_channel_get(void)
{
	return monitor_channel;
}
void monitor_channel_set(MONITOR_CH ch)
{
	monitor_channel = ch;
	extern void tuya_current_channel_set(int channel);
	tuya_current_channel_set(monitor_channel);
}
static AUDIO_TALK_PATTERN audio_talk_status = AI_AO_C;
static bool audiocast = false;

void audio_send_task_open(audio_talk_ctrl *ctrl)
{
	if (ctrl->option & (1 << AUDIO_SEND_EN))
	{
		int eth_p_id = 0;
		int slave_id = 0;
		slave_id = network_get_id_slave_id(network_local_device_get(), ctrl->user.dev_id);
		eth_p_id = network_common_socket_eth_p_get(1, slave_id, ctrl->user.family_id);
		Debug_Lib("%d ,%d ,%d ,0x%x\n\n\n", ctrl->user.dev_id, ctrl->user.family_id, slave_id, eth_p_id);

		if (network_audio_send_task_eth_id() != eth_p_id)
		{
			network_audio_send_package_close();
			network_audio_send_package_open(eth_p_id);
		}
	}

	return;
}
void audio_receive_task_open(audio_talk_ctrl *ctrl)
{
	if (ctrl->option & (1 << AUDIO_RECEIVE_EN))
	{
		int eth_p_id = 0;
		int slave_id = 0;
		slave_id = network_get_id_slave_id(network_local_device_get(), ctrl->user.dev_id);
		eth_p_id = network_common_socket_eth_p_get(1, slave_id, ctrl->user.family_id);
		Debug_Lib("%d ,%d ,%d ,0x%x\n\n\n", ctrl->user.dev_id, ctrl->user.family_id, slave_id, eth_p_id);

		if (network_audio_receive_task_eth_id() != eth_p_id)
		{
			network_audio_receive_package_close();
			network_audio_receive_package_open(eth_p_id);
		}
	}

	return;
}
void audio_decode_task_open(audio_talk_ctrl *ctrl)
{
	Debug_Lib("\n\n\n");

	if (ctrl->option & (1 << AUDIO_OUT_EN))
	{
		extern void audio_data_decode_power(bool enable);
		audio_decode_open(ctrl->vol);
		audio_data_decode_power(true);
		audio_decode_start();
		extern bool audio_output_volume_set(int vol);
		audio_output_volume_set(ctrl->vol);
	}
	else
	{
		audio_decode_stop();
	}
}
void audio_input_task_open(audio_talk_ctrl *ctrl)
{
	Debug_Lib("\n\n\n");
	if (ctrl->option & (1 << AUDIO_IN_EN))
	{
		audio_input_open();
		audio_input_start();
	}
	else
	{
		audio_input_stop();
	}
}

static void (*audio_operation_func[])(audio_talk_ctrl *ctrl) =
	{
		audio_send_task_open,
		audio_receive_task_open,
		audio_decode_task_open,
		audio_input_task_open,
};
#if 1
bool audio_talk_open(audio_talk_ctrl ctrl)
{
	Debug_Lib("%s ----------> [device]:%d   [talk_mode] %d [open_audiocast] :%d   [audio_decode]:%d  [family_id]:%d\n\r", __func__, ctrl.user.dev_id, ctrl.talk_pattern, ctrl.open_audiocast, ctrl.audio_decode, ctrl.user.family_id);

	if (ctrl.user.dev_id == DEVICE_CCTV_1 || ctrl.user.dev_id == DEVICE_CCTV_2)
	{
		return false;
	}

	int audio_operation_num = sizeof(audio_operation_func) / sizeof(audio_operation_func[0]);

	for (int i = 0; i < audio_operation_num; i++)
	{
		if (audio_operation_func[i])
		{
			audio_operation_func[i](&ctrl);
		}
	}
	audio_talk_status = ctrl.talk_pattern;
	return true;
}
// talk_mode : 0 -> ai close  ao close
//			  1 -> ai open  ao close
//			  2 -> ai close  ao open
//			  3 -> ai open  ao open
#else
bool audio_talk_open(audio_talk_ctrl ctrl)
{

	Debug_Lib("%s ----------> [device]:%d   [talk_mode] %d [open_audiocast] :%d   [audio_decode]:%d  [family_id]:%d\n\r", __func__, ctrl.user.dev_id, ctrl.talk_pattern, ctrl.open_audiocast, ctrl.audio_decode, ctrl.user.family_id);
	extern void audio_data_decode_power(bool enable);

	if (ctrl.user.dev_id == DEVICE_CCTV_1 || ctrl.user.dev_id == DEVICE_CCTV_2)
	{
		return false;
	}

	if (audiocast == false && ctrl.open_audiocast == true)
	{
		if (ctrl.user.dev_id == network_local_device_get())
		{
			return false;
		}

		audiocast = ctrl.open_audiocast;

		audio_send_task_open(&(ctrl.user));
		audio_receive_task_open(&(ctrl.user));

		audio_decode_open(ctrl.vol);
	}

	audio_data_decode_power(ctrl.audio_decode);
	Debug_Lib("%s ----------> [DEVICE]:%d   [talk_mode] %d [open_audiocast] :%d   [audio_decode]:%d\n\r", __func__, ctrl.user.dev_id, ctrl.talk_pattern, ctrl.open_audiocast, ctrl.audio_decode);
	if (ctrl.talk_pattern == AI_AO_C)
	{
		audio_input_stop();
		audio_decode_stop();
		Debug_Lib("%s=======AI_AO_C=========:::>>\n\r", __func__);
	}
	else if (ctrl.talk_pattern == AI_O_AO_C)
	{
		audio_input_start();
		audio_decode_stop();
		Debug_Lib("%s=======AI_O_AO_C=========:::>>\n\r", __func__);
	}
	else if (ctrl.talk_pattern == AI_C_AO_O)
	{
		audio_decode_start();
		audio_input_stop();
		Debug_Lib("%s=======AI_C_AO_O=========:::>>%d\n\r", __func__, __LINE__);
	}
	else if (ctrl.talk_pattern == AI_AO_O)
	{
		audio_input_open();
		audio_input_start();
		audio_decode_start();
		Debug_Lib("%s==========AI_AO_O======:::>>\n\r", __func__);
	}

	audio_talk_status = ctrl.talk_pattern;
	return true;
}
#endif

bool audio_talk_close(bool all_close)
{
	Debug_Lib("%s <<========================:%d\n", __func__, audiocast);
	// if (audiocast == false)
	// {
	// 	Debug_Lib("device audio not talk \n");
	// 	return false;
	// }
	audiocast = false;
	network_audio_send_package_close();
	network_audio_receive_package_close();

	if (all_close)
	{
		audio_input_close();
		audio_decode_close();

		audio_talk_status = AI_AO_C;
	}
	return true;
}

AUDIO_TALK_PATTERN is_audio_talk_open(void)
{
	return audio_talk_status;
}

bool monitor_valid_channel_check(char channel)
{
	if (channel == MON_CH_DOOR_1 && device_online_state_get(DEVICE_OUTDOOR_1) && device_enable_state_get(DEVICE_OUTDOOR_1))
	{
		return true;
	}
	else if (channel == MON_CH_DOOR_2 && device_online_state_get(DEVICE_OUTDOOR_2) && device_enable_state_get(DEVICE_OUTDOOR_2))
	{
		return true;
	}
	else if (channel == MON_CH_CCTV_1 && device_enable_state_get(DEVICE_CCTV_1) && device_online_state_get(DEVICE_CCTV_1))
	{
		return true;
	}
	else if (channel == MON_CH_CCTV_2 && device_enable_state_get(DEVICE_CCTV_2) && device_online_state_get(DEVICE_CCTV_2))
	{
		return true;
	}
	return false;
}
