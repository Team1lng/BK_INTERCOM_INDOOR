#include "user_data.h"
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "stdio.h"
#include "stdlib.h"
#include "layout_define.h"
#include "ak_drv_wdt.h"

static user_data_info user_data = {0};

static user_data_info user_data_default = {
	.app_versions = APP_VERSION_V,

	.system_mute = false,
	.motion.enable = true,
	.motion.record_flag = 0,
	.auto_record_mode = 0,

#ifdef PUBLIC_VERSION
	.user_pwd = "888888",
#else
	.user_pwd = "111111",
#endif

	.admin_sw = 0,

	.card_management = false,
	.fingetprint_management = false,
	.password_management = false,

	.audio.key_sound = true,
	.audio.ringback = true,
	.audio.door1_ring = 1,
	.audio.door2_ring = 2,
	.audio.door_ring_val = 5,

	.ring1.door1.timer_start = 6 * 100,
	.ring1.door2.timer_start = 6 * 100,
	.ring1.door1.timer_end = 12 * 100,
	.ring1.door2.timer_end = 12 * 100,

	.ring1.door1.ring_time = 30,
	.ring1.door2.ring_time = 30,

	.ring1.door1.ring_mode = false,
	.ring1.door2.ring_mode = false,
#ifdef INTERCOM_VERSION
	.ring1.door1.ring = 2,
	.ring1.door2.ring = 2,
#else
	.ring1.door1.ring = 1,
	.ring1.door2.ring = 1,
#endif
	.ring1.door1.custom_ring = 0,
	.ring1.door2.custom_ring = 0,
	.ring1.door1.ring_val = 5,
	.ring1.door2.ring_val = 5,

	.ring2.door1.timer_start = 12 * 100,
	.ring2.door2.timer_start = 12 * 100,
	.ring2.door1.timer_end = 18 * 100,
	.ring2.door2.timer_end = 18 * 100,

	.ring2.door1.ring_time = 30,
	.ring2.door2.ring_time = 30,

	.ring2.door1.ring_mode = false,
	.ring2.door2.ring_mode = false,
#ifdef INTERCOM_VERSION
	.ring2.door1.ring = 2,
	.ring2.door2.ring = 2,
#else
	.ring2.door1.ring = 1,
	.ring2.door2.ring = 1,
#endif
	.ring2.door1.custom_ring = 0,
	.ring2.door2.custom_ring = 0,
	.ring2.door1.ring_val = 5,
	.ring2.door2.ring_val = 5,

	.ring3.door1.timer_start = 18 * 100,
	.ring3.door2.timer_start = 18 * 100,
	.ring3.door1.timer_end = 6 * 100,
	.ring3.door2.timer_end = 6 * 100,

	.ring3.door1.ring_time = 30,
	.ring3.door2.ring_time = 30,

	.ring3.door1.ring_mode = false,
	.ring3.door2.ring_mode = false,
#ifdef INTERCOM_VERSION
	.ring3.door1.ring = 2,
	.ring3.door2.ring = 2,
#else
	.ring3.door1.ring = 1,
	.ring3.door2.ring = 1,
#endif
	.ring3.door1.custom_ring = 0,
	.ring3.door2.custom_ring = 0,
	.ring3.door1.ring_val = 5,
	.ring3.door2.ring_val = 5,

	.alarm.auto_record = false,
	.alarm.alarm_1_enable = false,
	.alarm.alarm_1_trigger = false,
	.alarm.alarm_2_enable = false,
	.alarm.alarm_2_trigger = false,

	.other.network_device = 1, // 1-4表示室内机ID1-ID4 5-6表示DOOR1-DOOR2,7-8表示CCTV1-CCTV2
	.other.family_id = 1,
	.other.screen_saver = 1, // 0 off,1:clock
	//.other.call_records = 0,
	//.other.info_records = 0,
	//.other.senser_records = 0,
	//.other.motion_records = 0,
	.other.MD_preview = 0,
	.other.date_format = 0, // 0:年-月-日, 1:月-日-年, 2:日-月-年

	.other.unlock_time = 2,
	.other.model = AT_HOME_PATTERN,
	.other.brightness = 3,
	.other.chime_type = true,
	.other.unlock_hint = false,

	.language.en = {true},
#ifdef PUBLIC_VERSION
#if defined(INTERCOM_VERSION)
	.language.index = DUTCH,
#else
	.language.index = ENGLISH,
#endif
#else
	.language.index = POLISH,
#endif

	.weather.weather_switch = true,
	.weather.duration_start = 0,
	.weather.duration_end = 2359,

	.wifi.wifi_open_flag = false,
	.wifi.wifi_connect_flag = false,
	.wifi.wifi_name = NULL,
	.wifi.wifi_pwd = NULL,

	.door1.enable_sw = 1,

	.door1.unlock_delay = 2,
	.door1.ungate1_delay = 2,

	.door1.record_mode = 0,
	.door1.motion_sw = 0,
	.door1.motion_duration = 10,
	.door1.motion_sensitivity = 0,
	.door1.motion_record_mode = 0,
	.door1.message_sw = false,
	.door1.message_time = 30,
	.door1.record_time = 60,
	.door1.talk_volume = 10,
	.door1.out_talk_volume = 10,
	.door1.brightness = 5,
	.door1.exit_button_lock = 0,
	.door1.exit_button_gate1 = 0,

	.door2.enable_sw = 1,

	.door2.unlock_delay = 2,
	.door2.ungate1_delay = 2,

	.door2.record_mode = 0,
	.door2.motion_sw = 0,
	.door2.motion_duration = 10,
	.door2.motion_sensitivity = 0,
	.door2.motion_record_mode = 0,
	.door2.message_sw = false,
	.door2.message_time = 30,
	.door2.record_time = 60,
	.door2.talk_volume = 10,
	.door2.out_talk_volume = 10,
	.door2.brightness = 5,
	.door2.exit_button_lock = 0,
	.door2.exit_button_gate1 = 0,

	.camera1.brightness = 5,
	.camera1.enable = true,
	.camera1.model = 0,
	.camera1.ip = "0.0.0.0",
	.camera1.account = "000000",
	.camera1.pwd = "000000",
	.camera1.url = "",

	.camera2.brightness = 5,
	.camera2.enable = true,
	.camera2.model = 0,
	.camera2.ip = "0.0.0.0",
	.camera2.account = "000000",
	.camera2.pwd = "000000",
	.camera2.url = "",

	.scene.digital_photo_frame_sw = 0,
	.scene.digital_photo_sw_time = 10,
	.scene.bg_music_sw = 0,
	.scene.bg_music_vol = 5,

	.tuya_info.tuya_uuid = {0},
	.tuya_info.tuya_key = {0},
	.tuya_info.index = 0,
	.tuya_info.used = false,
	.tuya_info.lock_id = false,

	.pairing_mode = WLAN_NET,
	.allocation_mode = STATIC_ALLOC,
};

#define user_data_check_range_out(cur, min, max)                           \
	if ((user_##data.cur < min) || (user_##data.cur > max))                \
	{                                                                      \
		printf("user data error %d(%d,%d) \n", user_##data.cur, min, max); \
		user_##data.cur = user_##data##_default.cur;                       \
	}

void app_version_create(int ver)
{
	int fd = open(LOCAL_APP_VER_PATH, O_WRONLY | O_CREAT, 0644);
	if (fd < 0)
	{
		Debug("write open %s fail \n", LOCAL_APP_VER_PATH);
		return;
	}

	write(fd, &ver, sizeof(int));

	close(fd);
	system("sync");
	return;
}

int local_app_version_get(void)
{
	int fd = open(LOCAL_APP_VER_PATH, O_RDONLY);
	if (fd < 0)
	{
		Debug("write open %s fail \n", LOCAL_APP_VER_PATH);
		return 0;
	}
	int ver = 0;
	read(fd, &ver, sizeof(int));

	close(fd);
	system("sync");
	printf("%s===============>>>%d\n", __func__, ver);
	return ver;
}

int sd_app_version_get(void)
{
	int fd = open(SD_APP_VER_PATH, O_RDONLY);
	if (fd < 0)
	{
		Debug("write open %s fail \n", SD_APP_VER_PATH);
		return 0;
	}
	int ver = 0;
	read(fd, &ver, sizeof(int));

	close(fd);
	system("sync");
	printf("%s===============>>>%d\n", __func__, ver);
	return ver;
}

bool app_updata_check(void)
{
	printf("%s APP_VERSION_V:%d,PREV VERSION:%d\n", __func__, APP_VERSION_V, user_data_get()->app_versions);
	if (user_data_get()->app_versions != APP_VERSION_V)
	{
		printf("\n\n*****************************************\n");
		printf("%s =========================>>>>>\n", __func__);
		printf("*****************************************\n");
		// user_data_reset();//版本不一样说明结构体也变动，再复位意义不大，直接删除源文件
		system("rm -rf " USER_DATA_PATH);
		system("sync");

		user_data = user_data_default;
		user_data_save();
		return true;
	}
	return false;
}

/***
** 日期: 2022-05-05 08:47
** 作者: leo.liu
** 函数作用：检验数据是否合法
** 返回参数说明：
***/
// static void user_data_check_valid(void)
// {
// 	user_data_check_range_out(system_mute,false,true);
// 	user_data_check_range_out(motion.enable,false,true);
// 	user_data_check_range_out(motion.record_flag,0,1);
// 	user_data_check_range_out(auto_record_mode,0,2);

// 	// user_data_check_range_out(user_pwd[0],'0','9');
// 	// user_data_check_range_out(user_pwd[1],'0','9');
// 	// user_data_check_range_out(user_pwd[2],'0','9');
// 	// user_data_check_range_out(user_pwd[3],'0','9');
// 	// user_data_check_range_out(user_pwd[4],'0','9');
// 	// user_data_check_range_out(user_pwd[5],'0','9');

// 	user_data_check_range_out(admin_sw,false,true);

// 	user_data_check_range_out(audio.key_sound,false,true);
// 	user_data_check_range_out(audio.ringback,false,true);
// 	user_data_check_range_out(audio.door1_ring,1,6);
// 	user_data_check_range_out(audio.door2_ring,1,6);
// 	user_data_check_range_out(audio.door_ring_val,0,10);
// 	user_data_check_range_out(audio.door_talk_val,0,10);

// 	user_data_check_range_out(ring1.door1.timer_start,0,23*100);
// 	user_data_check_range_out(ring1.door2.timer_start,0,23*100);
// 	user_data_check_range_out(ring1.door1.timer_end,0,23*100);
// 	user_data_check_range_out(ring1.door2.timer_end,0,23*100);
// 	user_data_check_range_out(ring1.door1_ring_time,10,45);
// 	user_data_check_range_out(ring1.door1_ring_time,10,45);
// 	user_data_check_range_out(ring1.door1_ring_mode,false,true);
// 	user_data_check_range_out(ring1.door2_ring_mode,false,true);
// 	user_data_check_range_out(ring1.door1_ring,1,6);
// 	user_data_check_range_out(ring1.door2_ring,1,6);
// 	user_data_check_range_out(ring1.door1_custom_ring,0,99);
// 	user_data_check_range_out(ring1.door2_custom_ring,0,99);
// 	user_data_check_range_out(ring1.door1_ring_val,0,10);
// 	user_data_check_range_out(ring1.door1_ring_val,0,10);

// 	user_data_check_range_out(ring2.door1.timer_start,0,23*100);
// 	user_data_check_range_out(ring2.door2.timer_start,0,23*100);
// 	user_data_check_range_out(ring2.door1.timer_end,0,23*100);
// 	user_data_check_range_out(ring2.door2.timer_end,0,23*100);
// 	user_data_check_range_out(ring2.door1_ring_time,10,45);
// 	user_data_check_range_out(ring2.door1_ring_time,10,45);
// 	user_data_check_range_out(ring2.door1_ring_mode,false,true);
// 	user_data_check_range_out(ring2.door2_ring_mode,false,true);
// 	user_data_check_range_out(ring2.door1_ring,1,6);
// 	user_data_check_range_out(ring2.door2_ring,1,6);
// 	user_data_check_range_out(ring2.door1_custom_ring,0,99);
// 	user_data_check_range_out(ring2.door2_custom_ring,0,99);
// 	user_data_check_range_out(ring2.door1_ring_val,0,10);
// 	user_data_check_range_out(ring2.door1_ring_val,0,10);

// 	user_data_check_range_out(ring3.door1.timer_start,0,23*100);
// 	user_data_check_range_out(ring3.door2.timer_start,0,23*100);
// 	user_data_check_range_out(ring3.door1.timer_end,0,23*100);
// 	user_data_check_range_out(ring3.door2.timer_end,0,23*100);
// 	user_data_check_range_out(ring3.door1_ring_time,10,45);
// 	user_data_check_range_out(ring3.door1_ring_time,10,45);
// 	user_data_check_range_out(ring3.door1_ring_mode,false,true);
// 	user_data_check_range_out(ring3.door2_ring_mode,false,true);
// 	user_data_check_range_out(ring3.door1_ring,1,6);
// 	user_data_check_range_out(ring3.door2_ring,1,6);
// 	user_data_check_range_out(ring3.door1_custom_ring,0,99);
// 	user_data_check_range_out(ring3.door2_custom_ring,0,99);
// 	user_data_check_range_out(ring3.door1_ring_val,0,10);
// 	user_data_check_range_out(ring3.door1_ring_val,0,10);

// 	user_data_check_range_out(alarm.auto_record,false,true);
// 	user_data_check_range_out(alarm.alarm_1_enable,false,true);
// 	user_data_check_range_out(alarm.alarm_1_trigger,false,true);
// 	user_data_check_range_out(alarm.alarm_2_enable,false,true);
// 	user_data_check_range_out(alarm.alarm_2_trigger,false,true);

// 	user_data_check_range_out(other.network_device,1,6);
// 	user_data_check_range_out(other.family_id,1,4);
// 	user_data_check_range_out(other.screen_saver,0,1);
// 	user_data_check_range_out(other.MD_preview,false,true);
// 	user_data_check_range_out(other.date_format,0,2);
// 	user_data_check_range_out(language.index,ENGLISH,ENGLISH);
// 	user_data_check_range_out(other.unlock_time,1,10);
// 	user_data_check_range_out(other.model,0,2);
// 	user_data_check_range_out(other.brightness,1,5);

// 	user_data_check_range_out(wifi.wifi_open_flag,false,true);
// 	user_data_check_range_out(wifi.wifi_connect_flag,false,true);
// 	// user_data_check_range_out(wifi.wifi_name,NULL,NULL);
// 	// user_data_check_range_out(wifi.wifi_pwd,NULL,NULL);

// 	user_data_check_range_out(door1.enable_sw,false,true);
// 	user_data_check_range_out(door1.unlock_delay,1,10);
// 	user_data_check_range_out(door1.ungate1_delay,1,10);
// 	user_data_check_range_out(door1.record_mode,false,true);
// 	user_data_check_range_out(door1.motion_sw,false,true);
// 	user_data_check_range_out(door1.motion_duration,10,300);
// 	user_data_check_range_out(door1.motion_sensitivity,0,3);
// 	user_data_check_range_out(door1.motion_record_mode,0,1);
// 	user_data_check_range_out(door1.message_sw,false,true);
// 	user_data_check_range_out(door1.message_time,30,120);
// 	user_data_check_range_out(door1.record_time,30,120);
// 	user_data_check_range_out(door1.talk_volume,0,10);

// 	user_data_check_range_out(door2.enable_sw,false,true);
// 	user_data_check_range_out(door2.unlock_delay,1,10);
// 	user_data_check_range_out(door2.ungate1_delay,1,10);
// 	user_data_check_range_out(door2.record_mode,false,true);
// 	user_data_check_range_out(door2.motion_sw,false,true);
// 	user_data_check_range_out(door2.motion_duration,10,300);
// 	user_data_check_range_out(door2.motion_sensitivity,0,3);
// 	user_data_check_range_out(door2.motion_record_mode,0,1);
// 	user_data_check_range_out(door2.message_sw,false,true);
// 	user_data_check_range_out(door2.message_time,30,120);
// 	user_data_check_range_out(door2.record_time,30,120);
// 	user_data_check_range_out(door2.talk_volume,0,10);

// 	user_data_check_range_out(camera1.enable,false,true);
// 	user_data_check_range_out(camera1.model,false,true);

// 	user_data_check_range_out(camera2.enable,false,true);
// 	user_data_check_range_out(camera2.model,false,true);

// 	user_data_check_range_out(scene.digital_photo_frame_sw,false,true);
// 	user_data_check_range_out(scene.digital_photo_sw_time,1,30);
// 	user_data_check_range_out(scene.bg_music_sw,false,true);
// 	user_data_check_range_out(scene.bg_music_vol,1,10);

// 	user_data_check_range_out(tuya_info.index,0,65535);
// 	user_data_check_range_out(tuya_info.used,false,true);
// 	user_data_check_range_out(tuya_info.lock_id,false,true);

// 	user_data_check_range_out(pairing_mode,WLAN_NET,WIRED_NET);
// }

user_ring_info ring_attr;
void ring_init(void)
{
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	int curr_time = tm.tm_hour * 100 + tm.tm_min;

	if ((user_data.ring1.door1.timer_start > user_data.ring1.door1.timer_end) && (curr_time > user_data.ring1.door1.timer_start || curr_time < user_data.ring1.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring1.door1;
	}
	else if ((user_data.ring1.door1.timer_start < user_data.ring1.door1.timer_end) && (curr_time > user_data.ring1.door1.timer_start && curr_time < user_data.ring1.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring1.door1;
	}
	else if ((user_data.ring2.door1.timer_start > user_data.ring2.door1.timer_end) && (curr_time > user_data.ring2.door1.timer_start || curr_time < user_data.ring2.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring2.door1;
	}
	else if ((user_data.ring2.door1.timer_start < user_data.ring2.door1.timer_end) && (curr_time > user_data.ring2.door1.timer_start && curr_time < user_data.ring2.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring2.door1;
	}
	else if ((user_data.ring3.door1.timer_start > user_data.ring3.door1.timer_end) && (curr_time > user_data.ring3.door1.timer_start || curr_time < user_data.ring3.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring3.door1;
	}
	else if ((user_data.ring3.door1.timer_start < user_data.ring3.door1.timer_end) && (curr_time > user_data.ring3.door1.timer_start && curr_time < user_data.ring3.door1.timer_end))
	{
		ring_attr.door1 = user_data.ring3.door1;
	}

	if ((user_data.ring1.door2.timer_start > user_data.ring1.door2.timer_end) && (curr_time > user_data.ring1.door2.timer_start || curr_time < user_data.ring1.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring1.door2;
	}
	else if ((user_data.ring1.door2.timer_start < user_data.ring1.door2.timer_end) && (curr_time > user_data.ring1.door2.timer_start && curr_time < user_data.ring1.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring1.door2;
	}
	else if ((user_data.ring2.door2.timer_start > user_data.ring2.door2.timer_end) && (curr_time > user_data.ring2.door2.timer_start || curr_time < user_data.ring2.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring2.door2;
	}
	else if ((user_data.ring2.door2.timer_start < user_data.ring2.door2.timer_end) && (curr_time > user_data.ring2.door2.timer_start && curr_time < user_data.ring2.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring2.door2;
	}
	else if ((user_data.ring3.door2.timer_start > user_data.ring3.door2.timer_end) && (curr_time > user_data.ring3.door2.timer_start || curr_time < user_data.ring3.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring3.door2;
	}
	else if ((user_data.ring3.door2.timer_start < user_data.ring3.door2.timer_end) && (curr_time > user_data.ring3.door2.timer_start && curr_time < user_data.ring3.door2.timer_end))
	{
		ring_attr.door2 = user_data.ring3.door2;
	}
}

// void ring_init(void){
// 	time_t seconds = time(NULL);
//     struct tm tm = {0};
//     localtime_r(&seconds, &tm);
// 	int start_time_1 = (user_data.ring1.door1.timer_start/100)*60 + user_data.ring1.door1.timer_start%100;
// 	int end_time_1 = (user_data.ring1.door1.timer_end/100)*60 + user_data.ring1.door1.timer_end%100;
// 	int start_time_2 = (user_data.ring2.door1.timer_start/100)*60 + user_data.ring2.door1.timer_start%100;
// 	int end_time_2 = (user_data.ring2.door1.timer_end/100)*60 + user_data.ring2.door1.timer_end%100;
// 	int start_time_3 = (user_data.ring3.door1.timer_start/100)*60 + user_data.ring3.door1.timer_start%100;
// 	int end_time_3 = (user_data.ring3.door1.timer_end/100)*60 + user_data.ring3.door1.timer_end%100;
// 	int cur_time = tm.tm_hour * 60 + tm.tm_min;

// 	if((cur_time >= start_time_1 && cur_time <= end_time_1)&&(start_time_1 < end_time_1)){
// 		Debug("11----%d - %d--[%d]--------------->>%d\n\n",start_time_1,end_time_1,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring1.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring1.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring1.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring1.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring1.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}else if((!(cur_time >= end_time_1 && cur_time <= start_time_1))&&(start_time_1 > end_time_1)){
// 		Debug("11----%d - %d--[%d]--------------->>%d\n\n",start_time_1,end_time_1,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring1.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring1.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring1.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring1.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring1.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}else if((cur_time >= start_time_2 && cur_time <= end_time_2)&&start_time_2 < end_time_2){
// 		Debug("22----%d - %d--[%d]--------------->>%d\n\n",start_time_2,end_time_2,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring2.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring2.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring2.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring2.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring2.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}else if((!(cur_time >= end_time_2 && cur_time <= start_time_2))&&(start_time_2 > end_time_2)){
// 		Debug("22----%d - %d--[%d]--------------->>%d\n\n",start_time_2,end_time_2,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring2.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring2.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring2.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring2.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring2.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}else if((cur_time >= start_time_3 && cur_time <= end_time_3)&&start_time_3 < end_time_3){
// 		Debug("33----%d - %d--[%d]--------------->>%d\n\n",start_time_3,end_time_3,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring3.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring3.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring3.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring3.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring3.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}else if((!(cur_time >= end_time_3 && cur_time <= start_time_3))&&(start_time_3 > end_time_3)){
// 		Debug("33----%d - %d--[%d]--------------->>%d\n\n",start_time_3,end_time_3,cur_time,__LINE__);
// 		ring_attr.door1_ring = user_data.ring3.door1_ring;
// 		ring_attr.door1_ring_val = user_data.ring3.door1_ring_val;
// 		ring_attr.door1_ring_time = user_data.ring3.door1_ring_time;
// 		ring_attr.door1_ring_mode = user_data.ring3.door1_ring_mode;
// 		ring_attr.door1_custom_ring = user_data.ring3.door1_custom_ring;
// 	Debug("-------ring_attr.door1_ring = %d-------->>>>\n",ring_attr.door1_ring);
// 	Debug("-------ring_attr.door1_ring_val = %d-------->>>>\n",ring_attr.door1_ring_val);
// 	Debug("-------ring_attr.door1_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door1_ring_time);
// 	}
// 	start_time_1 = (user_data.ring1.door2.timer_start/100)*60 + user_data.ring1.door2.timer_start%100;
// 	end_time_1 = (user_data.ring1.door2.timer_end/100)*60 + user_data.ring1.door2.timer_end%100;
// 	start_time_2 = (user_data.ring2.door2.timer_start/100)*60 + user_data.ring2.door2.timer_start%100;
// 	end_time_2 = (user_data.ring2.door2.timer_end/100)*60 + user_data.ring2.door2.timer_end%100;
// 	start_time_3 = (user_data.ring3.door2.timer_start/100)*60 + user_data.ring3.door2.timer_start%100;
// 	end_time_3 = (user_data.ring3.door2.timer_end/100)*60 + user_data.ring3.door2.timer_end%100;

// 	if((cur_time >= start_time_1 && cur_time <= end_time_1)&&(start_time_1 < end_time_1)){
// 		Debug("11----%d - %d--[%d]--------------->>%d\n\n",start_time_1,end_time_1,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring1.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring1.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring1.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring1.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring1.door2_custom_ring;
// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}else if((!(cur_time >= end_time_1 && cur_time <= start_time_1))&&(start_time_1 > end_time_1)){
// 		Debug("11----%d - %d--[%d]--------------->>%d\n\n",start_time_1,end_time_1,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring1.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring1.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring1.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring1.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring1.door2_custom_ring;

// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}else if((cur_time >= start_time_2 && cur_time <= end_time_2)&&start_time_2 < end_time_2){
// 		Debug("22----%d - %d--[%d]--------------->>%d\n\n",start_time_2,end_time_2,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring2.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring2.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring2.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring2.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring2.door2_custom_ring;
// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}else if((!(cur_time >= end_time_2 && cur_time <= start_time_2))&&(start_time_2 > end_time_2)){
// 		Debug("22----%d - %d--[%d]--------------->>%d\n\n",start_time_2,end_time_2,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring2.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring2.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring2.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring2.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring2.door2_custom_ring;
// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}else if((cur_time >= start_time_3 && cur_time <= end_time_3)&&start_time_3 < end_time_3){
// 		Debug("33----%d - %d--[%d]--------------->>%d\n\n",start_time_3,end_time_3,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring3.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring3.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring3.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring3.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring3.door2_custom_ring;
// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}else if((!(cur_time >= end_time_3 && cur_time <= start_time_3))&&(start_time_3 > end_time_3)){
// 		Debug("33----%d - %d--[%d]--------------->>%d\n\n",start_time_3,end_time_3,cur_time,__LINE__);
// 		ring_attr.door2_ring = user_data.ring3.door2_ring;
// 		ring_attr.door2_ring_val = user_data.ring3.door2_ring_val;
// 		ring_attr.door2_ring_time = user_data.ring3.door2_ring_time;
// 		ring_attr.door2_ring_mode = user_data.ring3.door2_ring_mode;
// 		ring_attr.door2_custom_ring = user_data.ring3.door2_custom_ring;

// 		Debug("-------ring_attr.door2_ring = %d-------->>>>\n",ring_attr.door2_ring);
// 	Debug("-------ring_attr.door2_ring_val = %d-------->>>>\n",ring_attr.door2_ring_val);
// 	Debug("-------ring_attr.door2_ring_time = %d-------->>>>\n\n\n\n\n\n\n",ring_attr.door2_ring_time);
// 	}

// }

void user_language_control(void)
{
	bool en[] = LANGUAGE_EN_GROUP;
	for (int i = 0; i < sizeof(en); i++)
	{
		// Debug("index:%d,en:%d\n", i, en[i]);
		user_data.language.en[i] = en[i];
	}
}

static bool user_data_save_flag = false;
bool user_data_save(void)
{
	user_data_save_flag = true;
	return true;
}

static bool watch_dog_flag = false;
void watchdog_open(void)
{
	if (watch_dog_flag)
	{
		return;
	}
	ak_drv_wdt_open(10);
	watch_dog_flag = true;
}

void watch_dog_close(void)
{
	if (watch_dog_flag)
	{
		ak_drv_wdt_close();
		watch_dog_flag = false;
	}
}

void watch_dog_feed(void)
{
	if (watch_dog_flag == true)
	{
		ak_drv_wdt_feed();
	}
}

void wlan0_health_handler(void *data)
{
	printf("Reload the WiFi module.... \n");
	system("/app/app/hi3881_reload.sh");
}

static void *user_data_task(void *arg)
{
	struct ak_timeval tv1, tv2, tv3;

	ak_get_ostime(&tv1);
	ak_get_ostime(&tv3);
	// watchdog_open();  // 看门狗开启及喂狗移到 layout_define.c 的 feed_watchdog_task
	while (1)
	{
		if (user_data_save_flag)
		{
			int fd = open(USER_DATA_PATH, O_WRONLY | O_CREAT, 0644);
			if (fd < 0)
			{
				Debug("write open %s fail \n", USER_DATA_PATH);
			}
			else
			{
				write(fd, &user_data, sizeof(user_data_info));

				close(fd);

				system("sync");
			}
			user_data_save_flag = false;
		}

		ak_get_ostime(&tv2);
		if ((tv2.sec - tv1.sec) > 3 * 60)
		{
			tv1 = tv2;
			system("sync");
			system("echo 3 > /proc/sys/vm/drop_caches");
		}

		if (abs(tv2.sec - tv3.sec) > 1)
		{
			// watch_dog_feed();  // 喂狗移到 layout_define.c 的 feed_watchdog_task
			tv3 = tv2;
		}

		if (wifi_usb_module_enable() &&
			current_layout_get() != &layout_add_wifi &&
			current_layout_get() != &layout_setting_wifi &&
			current_layout_get() != &layout_connect_wifi && 
			!is_jpg_record_ing() && 
			!is_video_recording())
		{
			extern int Wlan0HealthCheck(int interval, int error_count, void (*error_handler)(void *));
			int status = Wlan0HealthCheck(2000, 5, wlan0_health_handler);
			if (status)
			{
				user_data_get()->wifi.wifi_connect_flag = status == 1 ? 1 : 0;
			}
		}

		ak_sleep_ms(1 * 1000);
	}
	ak_thread_exit();
	return NULL;
}

bool user_data_init(void)
{
	ak_pthread_t pthread_id;

	monitor_device_init(&user_data_get()->door1, &user_data_get()->door2, &user_data_get()->camera1, &user_data_get()->camera2);

	int fd = open(USER_DATA_PATH, O_RDONLY);
	if (fd < 0)
	{
		user_data = user_data_default;
		user_data_save();
		Debug("read open %s fail \n", USER_DATA_PATH);
		ring_init();
		goto JIGE;
	}

	read(fd, &user_data, sizeof(user_data_info));
	// printf("ret:%d =================== size:%d\n",ret,sizeof(user_data_info));

	close(fd);
	app_updata_check();

	if ((user_data.other.network_device < 1) || (user_data.other.network_device > 6))
	{
		user_data.other.network_device = 1;
	}

	ring_init();

	memset(&(user_data_get()->tuya_info), 0, sizeof(user_data_get()->tuya_info));

	tuya_set_current_language(user_data_get()->language.index);
	network_local_family_set(user_data_get()->other.family_id);
	// extern void set_tuya_work_mode(unsigned int mode);
	set_tuya_work_mode(user_data_get()->other.model);

JIGE:
	user_language_control();
	ak_thread_create(&pthread_id, user_data_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

	return true;
}

user_data_info *user_data_get(void)
{
	return &user_data;
}
const user_data_info *user_default_data_get(void)
{
	return &user_data_default;
}

void user_data_reset(void)
{

	printf("*********************************>>>\n");
	printf("%s=====================>>>\n", __func__);
	printf("*********************************>>>\n");
	memcpy(user_data_default.tuya_info.tuya_uuid, user_data.tuya_info.tuya_uuid, sizeof(user_data.tuya_info.tuya_uuid));
	memcpy(user_data_default.tuya_info.tuya_key, user_data.tuya_info.tuya_key, sizeof(user_data.tuya_info.tuya_key));
	system("rm -rf " USER_DATA_PATH);
	system("sync");

	// user_net_pairing pairing_mode = user_data.pairing_mode;
	// char  tuya_uuid[IPC_UUID_LEN] = {0};
	// char tuya_key[IPC_AUTH_KEY_LEN] = {0};

	// memcpy(tuya_uuid,user_data.tuya_info.tuya_uuid,IPC_UUID_LEN);
	// memcpy(tuya_key,user_data.tuya_info.tuya_key,IPC_AUTH_KEY_LEN);
	user_data = user_data_default;

	// user_data.pairing_mode = pairing_mode;
	// memcpy(user_data.tuya_info.tuya_uuid,tuya_uuid,IPC_UUID_LEN);
	// memcpy(user_data.tuya_info.tuya_key,tuya_key,IPC_AUTH_KEY_LEN);

	ring_init();
	user_language_control();
	user_data_save();

	network_local_family_set(user_data_get()->other.family_id);
	network_local_device_set(user_data_get()->other.network_device);
	network_local_mac_set();
}
