#ifndef _USER_DATA_H_
#define _USER_DATA_H_
#include "stdbool.h"
#include "tuya_ipc_api.h"
#include "tuya_uuid_and_key.h"
#include "leo_api.h"

#define USER_DATA_PATH USER_FILE_PATH "user_data.cfg"
#define LOCAL_APP_VER_PATH USER_FILE_PATH "app_version.cfg"
#define SD_APP_VER_PATH "/tmp/app_version.cfg"

#define APP_VERSION_V 7

typedef enum
{
	AT_HOME_PATTERN,	 // 在家
	NOT_AT_HOME_PATTERN, // 离家
	MUTE_PATTERN,		 // 休眠
	PATTERN_TOTAL
} USER_WORK_PATTERN; // 工作模式    离家 、在家、休眠

typedef struct
{
	bool enable;
	char record_flag; // 0:photo,1:video;
} user_motion_info;

typedef struct
{
	bool key_sound;
	bool ringback;
	int door1_ring;
	int door2_ring;

	int door_ring_val;
} user_audio_info;

typedef struct
{
	int timer_start;
	int timer_end;

	int ring_time;

	int ring_mode;

	int ring;
	int custom_ring;

	int ring_val;
} door_ring_info;

typedef struct
{
	/* 	int door1_timer_start;
		int door1_timer_end;
		int door2_timer_start;
		int door2_timer_end;

		int door1_ring_time;
		int door2_ring_time;

		int door1_ring_mode;
		int door2_ring_mode;

		int door1_ring;
		int door2_ring;
		int door1_custom_ring;
		int door2_custom_ring;

		int door1_ring_val;
		int door2_ring_val; */
	door_ring_info door1;
	door_ring_info door2;
} user_ring_info;
user_ring_info ring_attr;

typedef struct
{
	bool auto_record;

	bool alarm_1_enable;
	bool alarm_1_trigger;

	bool alarm_2_enable;
	bool alarm_2_trigger;

} user_alarm_info;

typedef struct
{
	bool weather_switch;

	int duration_start;
	int duration_end;
} user_weather_info;

typedef struct
{
	int network_device;
	int family_id;
	char screen_saver;
	int date_format;

	// char call_records;
	// char info_records;
	// char senser_records;
	// char motion_records;
	bool MD_preview;
	int language;
	char unlock_time;
	USER_WORK_PATTERN model;

	int brightness;

	bool chime_type;
	char exit_burron;

	bool unlock_hint;
} user_other_info;

typedef struct
{
	bool en[32];
	int index;
} user_language_info;

typedef struct
{
	bool wifi_open_flag; // wifi打开标志位 0:关闭 1:打开
	bool wifi_connect_flag;

	char *wifi_name; // 已经连接的wifi名字
	char *wifi_pwd;	 // 已经连接的wifi密码
} user_wifi_info;

// typedef struct
// {
// 	bool enable_sw;
// 	int unlock_delay;
// 	int ungate1_delay;
// 	bool record_mode;
// 	bool motion_sw;
// 	int motion_sensitivity;
// 	int motion_record_mode;
// 	int motion_duration;
// 	bool message_sw;
// 	int message_time;
// 	int record_time;
// 	int talk_volume;
// }user_door_info;

// typedef struct
// {
// 	bool enable;
// 	bool model;
// 	char ip[16];
// 	char account[16];
// 	char pwd[16];

// }user_camera_info;

typedef struct
{
	bool digital_photo_frame_sw;
	int digital_photo_sw_time;
	bool bg_music_sw;
	int bg_music_vol;

} user_scene_info;

#define DATA_NUM_MAX 200
#define DATA_NUMBER_SIZE 5 // byte

typedef struct
{
	unsigned char lock_state;
	unsigned char number[DATA_NUMBER_SIZE];
} data_t;

typedef struct
{
	unsigned char total;
	data_t data[DATA_NUM_MAX];
} data_info_t;

typedef struct
{
	int app_versions;

	bool system_mute;
	user_motion_info motion;
	char user_pwd[7];
	char auto_record_mode; // 0:off,1:photo,2:video;

	bool admin_sw;

	bool card_management;

	bool fingetprint_management;

	bool password_management;

	bool date_time_display;

	user_audio_info audio;

	user_alarm_info alarm;

	user_other_info other;

	user_language_info language;

	user_weather_info weather;

	user_wifi_info wifi;

	door_info door1;
	door_info door2;

	camera_info camera1;
	camera_info camera2;

	user_scene_info scene;

	user_ring_info ring1;
	user_ring_info ring2;
	user_ring_info ring3;

	tuya_conf_info tuya_info;

	user_net_pairing pairing_mode;
	user_ip_allocation allocation_mode;
} user_data_info;

bool user_data_save(void);
bool user_data_init(void);
void ring_init(void);

user_data_info *user_data_get(void);
const user_data_info *user_default_data_get(void);
void user_data_reset(void);

int local_app_version_get(void);
#endif
