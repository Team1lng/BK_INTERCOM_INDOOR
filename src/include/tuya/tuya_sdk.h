#ifndef TUYA_SDK_H
#define TUYA_SDK_H

#include <stdbool.h>
#include "tuya_cloud_types.h"
#include "tuya_ipc_api.h"

#ifndef IPC_APP_VERSION
#define IPC_APP_VERSION "2.1.4" // Firmware version displayed on TUYA APP
#endif

#ifndef IPC_APP_PID
#define IPC_APP_PID "8hbui777kxqebtpk" /* "ryncb1fh1j6wtlbd" */
#endif

#define BCOMTECH_DP_LIGHT (138)
#define BCOMTECH_DP_DOORBELL (136)
#define BCOMTECH_DP_OUTDOOR_LOCK (148)
#define BCOMTECH_DP_INDORR_GATE2 (233)
#define BCOMTECH_DP_PICTURE (154)
#define BCOMTECH_DP_REBOOT_SYSTEM (162)
#define BCOMTECH_DP_ALARM_MSG (185)
#define BCOMTECH_DP_WORK_MODE (189)
#define BCOMTECH_DP_SWITCH_CHANNEL (231)
#define TUYA_DP_SD_STORAGE_ONLY_GET (109)
#define TUYA_DP_SD_STATUS_ONLY_GET (110)
#define TUYA_DP_SD_FORMAT (111)					/* Format sd card, BOOL type */
#define TUYA_DP_SD_FORMAT_STATUS_ONLY_GET (117) /* Format sd card status, VALUE type, -2000: SD card is being formatted, -2001: SD card formatting is abnormal,*/
#define BCOMTECH_DP_OUTDOOR_GATE1 (232)
#define BCOMTECH_DP_LOCK_SUPPORT (234)
#define BCOMTECH_DP_DEVICE_ACTIVE (235)
#define BCOMTECH_DP_DEVICE_SCREENSHOT (236)

typedef struct
{
	/*
		https://developer.tuya.com/cn/docs/iot/weather-function-description?id=Ka6dcs2cw4avp
	*/

	int temp;	   /* 温度 */
	int condition; /* 天气状况  阴/晴/雷/雨 */
	int thigh;	   /* 最高温 */
	int tlow;	   /* 最低温 */
} TUYA_API_WEATHER;

typedef enum
{
	/*切换监控*/
	TUYA_EVENT_MONITOR_SWAP,

	/*开灯*/
	TUYA_EVENT_LIGHT_ON,

	/*开锁*/
	TUYA_EVENT_OPEN_LOCK,

	/*开锁2*/
	TUYA_EVENT_OPEN_GATE1,

	/*开门*/
	TUYA_EVENT_OPEN_GATE2,

	/*模式切换*/
	TUYA_EVENT_WORK_MODE,

	/*Absent mode*/
	TUYA_EVENT_ABSENT_MODE,

	/*通话*/
	TUYA_EVENT_TALK,

	/*进入监控*/
	TUYA_EVENT_MONITOR_ENTER,

	/*退出监控*/
	TUYA_EVENT_MONITOR_QUIT,

	/* 截屏 */
	TUYA_EVENT_SCREENSHOT,

	/* 门钟检测 */
	TUYA_EVENT_DOOR_CHRIME,

	/* 涂鸦监控 */
	TUYA_EVENT_MONITOR_ING,

	/* 信箱检测 */
	TUYA_EVENT_MAILBOX_STATUS,

	/*MQTT 掉线*/
	TUYA_EVENT_MQTT_OFFLINE,
} tuya_event;

typedef struct
{
	int temp;
	int condition;
	int humidity;
	int pressure;
	int pm10;
	int pm25;
	int thigh; /* 最高温 */
	int tlow;  /* 最低温 */
	char city[32];
} tuya_api_weather;

OPERATE_RET http_gw_reset(IN CONST int stat);

VOID tuya_ipc_get_snapshot_cb(char *pjbuf, unsigned int *size);

tuya_api_weather *tuya_weather_get(void);

INT_T tuya_online_clinet_num_get(void);
void tuya_online_clinet_num_reset(void);

void tuya_ipc_ring_buffer_video_release_data(void); // 释放涂鸦队列音视频数据

bool wifi_work_restart(void); // 重新连接涂鸦服务器

bool tuya_wifi_sdk_init(const char *pid, char *uuid, char *key); // 初始化涂鸦库
bool is_online_tuya_cloud(void);

void tuya_current_channel_set(int channel); // 设置APP预览通道

void tuya_set_current_language(int language); // 设置上传APP的语言

int tuya_get_current_language(void); //

void tuya_language_total_get(int total);

int is_tuya_cloud_connected_num(void); // 当前APP连接室内机的数目

int tuya_switch_channel_upload_results(int channel); // 设置通道返回的结果

int tuya_channel_valid_report(void); // 上传有效通道到APP

int tuya_dp_138_response_light_switch(BOOL_T state);

int tuya_dp_148_response_accessory_lock(BOOL_T state); // 设置开锁状态

int tuya_dp_233_response_gate2(BOOL_T state); // 设置室内机开关门

int tuya_dp_189_response_work_mode(UINT_T mode);

int tuya_dp_232_response_abnormal_lock(BOOL_T state);

int tuya_dp_232_response_outdoor_gate1(BOOL_T state);

int tuya_dp_234_response_lock_support(void); // 离家模式

int tuya_dp_235_response_device_active(void); //

int tuya_dp_uploads_security_msg(char id, char *data, int size);

void update_tuya_lock_state(BOOL_T state);

void set_tuya_channel_state(int channel, bool state);

bool tuya_net_time_sync(struct tm *local_time);

IPC_REGISTER_STATUS tuya_ipc_register_status_get(void);

void tuya_network_dev_set(user_net_pairing *net_pairing);
#endif
