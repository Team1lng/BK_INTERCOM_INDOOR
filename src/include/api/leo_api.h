#ifndef _LEO_API_H_
#define _LEO_API_H_
#include <stdbool.h>
#include "stdlib.h"
#include "sys/time.h"
#include "network_common.h"

extern unsigned long long os_get_ms(void);
#define Debug_Lib (printf("\n\033[0;33;40m[***%s***]:[%u]:%llums\033[0m \t", __PRETTY_FUNCTION__, __LINE__, os_get_ms()), printf)

static inline unsigned long long get_sys_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
static inline unsigned long long get_sys_us(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

typedef enum REC_MODE
{
	REC_MODE_MANUAL = 1,
	REC_MODE_AUTO,
	REC_MODE_MOTION,
	REC_MODE_ALARM,
	REC_MODE_MESSAGE,
	REC_MODE_TUYA,
} record_mode_list;

enum audio_operation
{
	AUDIO_SEND_EN,
	AUDIO_RECEIVE_EN,
	AUDIO_OUT_EN,
	AUDIO_IN_EN,
};

#define REC_NOT_AUDIO 0X00
#define REC_OUT_AUDIO 0X01
#define REC_INT_AUDIO 0X02
#define REC_ALL_AUDIO 0X03

#define OPERATION_OPTION(x) (1 << (x))

#define CALL_REINGBACK_DISABLE (1 << 1)

typedef enum
{
	MONITOR_ENTER_NONE,
	MONITOR_ENTER_MANUAL,
	MONITOR_ENTER_CALL,
	MONITOR_ENTER_TUYA,
	MONITOR_ENTER_MONTION,
	MONITOR_ENTER_ALARM,
} MONITOR_ENTER_WAY;

typedef enum
{
	MON_CH_NONE,
	MON_CH_DOOR_1,
	MON_CH_DOOR_2,
	MON_CH_CCTV_1,
	MON_CH_CCTV_2,
	MON_CH_TOTAL
} MONITOR_CH;

typedef enum
{
	AI_AO_C,   // AI CLOSE			AO CLOSE
	AI_O_AO_C, // AI OPEN			 AO CLOSE
	AI_C_AO_O, // AI CLOSE			AO OPEN
	AI_AO_O,   // AI OPEN   		AO OPEN
	TALK_PATTERN_TOTAL
} AUDIO_TALK_PATTERN;

typedef struct
{
	bool enable_sw;
	int unlock_delay;
	int ungate1_delay;
	bool record_mode;
	bool motion_sw;
	int motion_sensitivity;
	int motion_record_mode;
	int motion_duration;
	bool message_sw;
	int message_time;
	int record_time;
	int talk_volume;
	int out_talk_volume;
	int brightness;
	int mailbox_num;
	bool exit_button_lock;
	bool exit_button_gate1;
} door_info;

typedef struct
{
	int brightness;
	bool enable;
	bool model;
	char ip[16];
	char account[16];
	char pwd[16];
	char url[128];
} camera_info;

typedef struct
{
	door_info *outdoor1;
	door_info *outdoor2;
	camera_info *cctv1;
	camera_info *cctv2;
} moniotr_config;

typedef struct
{
	network_device dev_id;
	int family_id;
} family_device;

typedef struct
{
	family_device user;

	char option;

	AUDIO_TALK_PATTERN talk_pattern;
	bool open_audiocast;
	bool audio_decode;
	int vol;
} audio_talk_ctrl;

typedef struct
{
	char type;
	char ch;
	bool is_video;
	int len;
	unsigned long long pts;

	unsigned char *data;
} record_data_node;

void leo_api_init(void);

void monitor_device_init(door_info *doo1, door_info *door2, camera_info *cctv1, camera_info *cctv2);

moniotr_config *monitor_config_get(void);

bool standby_timer_open(int timeout, void (*timeout_callback)(void));
bool standby_timer_close(void);

bool is_sdcard_insert(void);

void monitor_enter_way_set(MONITOR_ENTER_WAY flag);
MONITOR_ENTER_WAY monitor_enter_way_get(void);

void monitor_channel_set(MONITOR_CH ch);
void monitor_open(bool reset);
void monitor_switch(void);
void monitor_close(void);
void monitor_close_1(void);
MONITOR_CH monitor_channel_get(void);
bool monitor_valid_channel_check(char channel);

bool audio_talk_open(audio_talk_ctrl ctrl);
bool audio_talk_close(bool all_close);
AUDIO_TALK_PATTERN is_audio_talk_open(void);

bool record_pictrue_start(char mode, MONITOR_CH video_channel);

char record_video_type(void);

bool record_video_start(char mode, char audio_from, MONITOR_CH video_channel);

bool record_video_stop(char audio_flag);

bool media_thumb_device_open(int width, int height);

bool media_thumb_device_close(void);

bool media_thumb_load(int x, int y, int w, int h, const char *file_path);

bool video_play_open(const char *file);

bool video_play_stop(void);

bool video_play_pause(void);

char video_play_get_status(void); // 0:stop 1:working 2:pause

bool video_play_duration_get(int *cur, int *total);

void get_SD_space(unsigned long *bavail, unsigned long *disk_all_space);

int sd_free_space_insufficient(void);
#endif
