#ifndef _TUYA_SDK_H_
#define _TUYA_SDK_H_

#include <stdbool.h>
#include <time.h>
#include "./include/components/base_media/include/codec/tuya_g711_utils.h"

#define STREAM_CLIENT_MAX 4

#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

#define USER_LOG_LEVEL LOG_LEVEL_ERROR

#define _LOG_COLOR_WHITE "\033[0m"
#define _LOG_COLOR_RED "\033[0;31m"
#define _LOG_COLOR_GREEN "\033[0;32m"
#define _LOG_COLOR_YELLOW "\033[0;33m"
#define _LOG_COLOR_BLUE "\033[0;34m"
#define _LOG_COLOR_PINK "\033[0;35m"
#define _LOG_COLOR_CYAN "\033[0;36m"
#define _LOG_COLOR_NONE "\033[0m"

#if (USER_LOG_LEVEL >= LOG_LEVEL_ERROR)
#define db_log_error(format, ...) \
    printf(_LOG_COLOR_RED "\r[ERROR][%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define db_log_error(format, ...)
#endif

#if (USER_LOG_LEVEL >= LOG_LEVEL_WARN)
#define db_log_warn(format, ...) \
    printf(_LOG_COLOR_YELLOW "\r[WARNING][%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define db_log_warn(format, ...)
#endif

#if (USER_LOG_LEVEL >= LOG_LEVEL_DEBUG)
#define db_log_debug(format, ...) \
    printf(_LOG_COLOR_GREEN "\r[DEBUG][%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define db_log_debug(format, ...)
#endif

#if (USER_LOG_LEVEL >= LOG_LEVEL_INFO)
#define db_log_info(format, ...) \
    printf(_LOG_COLOR_WHITE "\r[INFO][%s:%04u]  " format _LOG_COLOR_NONE, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#else
#define db_log_info(format, ...)
#endif

#define WLAN_DEV "wlan0"
#define WIRED_DEV "eth2"

typedef enum
{
    WLAN_NET,  // wifi
    WIRED_NET, // 有限
    PAIRING_TOTAL
} user_net_pairing; //

typedef enum
{
    STATIC_ALLOC, // wifi
    UDHCPC_ALLOC, // 有限
    ALLOC_TOTAL
} user_ip_allocation;

typedef enum
{
    TUYA_DOOR_LOCK1 = 0,
    TUYA_DOOR_LOCK2,
    TUYA_INDOOR_LOCK
} tuya_door_lock_t;

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
    /*OTA升级*/
    TUYA_EVENT_OTA_UPGRADE,
} tuya_event;

typedef struct
{
    char pid[24];
    char uuid[36];
    char key[36];
    char ver[16];
    char net_dev[8];
    char cache_dir[64];
    char sd_dir[64];
} tuya_init_config_t;

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

typedef struct
{
    int ch;
    const char *name;
} tuya_ch_info_t;

/*******************************************************************
 * @brief  : tuya sdk 初始化
 *******************************************************************/
int tuya_sdk_init(const tuya_init_config_t *cfg);
/*******************************************************************
 * @brief  : tuya实时视频流推送至app
 *******************************************************************/
int tuya_realtime_video_put_frame(unsigned char *data, int size, unsigned long long pts);
/*******************************************************************
 * @brief  : tuya实时音频流推送至app
 *******************************************************************/
int tuya_realtime_audio_put_frame(unsigned char *data, int size, unsigned long long pts);
/*******************************************************************
 * @brief  : 获取tuya二维码短链接
 *******************************************************************/
const char *tuya_qrcode_shorturl_get(void);
/*******************************************************************
 * @brief  : 获取tuya在线状态
 *******************************************************************/
bool tuya_online_status_get(void);
/*******************************************************************
 * @brief  : 获取tuya观看视频流客户端数量
 *******************************************************************/
int tuya_client_num_get(void);
/*******************************************************************
 * @brief  : 呼叫tuya app并上报图片
 *******************************************************************/
int tuya_notify_call_event(int ch, const unsigned char *jpeg_buf, int size);
/*******************************************************************
 * @brief  : 上报报警图片
 *******************************************************************/
int tuya_notify_alarm_event(int ch, const unsigned char *jpeg_buf, int size);
/*******************************************************************
 * @brief  : 上报移动侦测照片
 *******************************************************************/
int tuya_notify_motion_event(int ch, const unsigned char *jpeg_buf, int size);
/*******************************************************************
 * @brief  : 同步网络时间
 *******************************************************************/
bool tuya_net_time_sync(struct tm *new_tm);
/*******************************************************************
 * @brief  : tuya环形缓冲区清理
 *******************************************************************/
void tuya_ring_buffer_clear(void);
/*******************************************************************
 * @brief  : g711a解码
 *******************************************************************/
static inline int tuya_g711a_decode(unsigned char *in_data, unsigned int in_len, unsigned char *out_data, unsigned int *out_len)
{
    return tuya_g711_decode(TUYA_G711_A_LAW, (short unsigned int *)in_data, in_len, out_data, out_len);
}
/*******************************************************************
 * @brief  : g711u解码
 *******************************************************************/
static inline int tuya_g711u_decode(unsigned char *in_data, unsigned int in_len, unsigned char *out_data, unsigned int *out_len)
{
    return tuya_g711_decode(TUYA_G711_MU_LAW, (short unsigned int *)in_data, in_len, out_data, out_len);
}
/*******************************************************************
 * @brief  : g711a编码
 *******************************************************************/
static inline int tuya_g711a_encode(unsigned char *in_data, unsigned int in_len, unsigned char *out_data, unsigned int *out_len)
{
    return tuya_g711_encode(TUYA_G711_A_LAW, (short unsigned int *)in_data, in_len, out_data, out_len);
}
/*******************************************************************
 * @brief  : g711u编码
 *******************************************************************/
static inline int tuya_g711u_encode(unsigned char *in_data, unsigned int in_len, unsigned char *out_data, unsigned int *out_len)
{
    return tuya_g711_encode(TUYA_G711_MU_LAW, (short unsigned int *)in_data, in_len, out_data, out_len);
}
/*******************************************************************
 * @brief  : 获取tuya天气信息
 *******************************************************************/
int tuya_door_lock_report(tuya_door_lock_t lock, bool state);
/*******************************************************************
 * @brief  : 获取tuya天气信息
 *******************************************************************/
tuya_api_weather *tuya_weather_get(void);
/*******************************************************************
 * @brief  : 多锁支持上报
 *******************************************************************/
int tuya_lock_support_report(void);
/*******************************************************************
 * @brief  : 截屏状态上报
 *******************************************************************/
int tuya_screenshot_report(bool state);
/*******************************************************************
 * @brief  : tuya通道信息上报
 *******************************************************************/
int tuya_channel_report(int curr_ch, tuya_ch_info_t *info, int total);
/*******************************************************************
 * @brief  : 本地存储是否正在录像
 *******************************************************************/
bool tuya_stream_storage_recording(void);
/*******************************************************************
 * @brief  : 本地存储是否已停止
 *******************************************************************/
bool tuya_stream_storage_stopped(void);
/*******************************************************************
 * @brief  : 本地存储录像开始
 *******************************************************************/
void tuya_stream_storage_start(void);
/*******************************************************************
 * @brief  : 本地存储录像停止
 *******************************************************************/
void tuya_stream_storage_stop(bool keep_upload);
/*******************************************************************
 * @brief  : 云存储录像开始
 *******************************************************************/
int tuya_cloud_storage_start(void);
/*******************************************************************
 * @brief  : 云存储录像停止
 *******************************************************************/
int tuya_cloud_storage_stop(void);
/*******************************************************************
 * @brief  : tuya设备解绑
 *******************************************************************/
void tuya_device_unbinding(void);
/*******************************************************************
 * @brief  : 上报子版本，即门口机版本
 *******************************************************************/
int tuya_sub_version_report(int channel, const char *version);

#endif