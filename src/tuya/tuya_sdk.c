#include <netdb.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_sdk_init.h"
#include "tuya_svc_netmgr_linkage.h"
#include "ty_sdk_common.h"
#include "ty_sdk_call.h"
#include "tal_system.h"
#include "tuya_sdk.h"
#include "tuya_video_frame_type.h"
#include "../include/anyka/ak_thread.h"
#include "../include/anyka/ak_common.h"
#include <stdatomic.h>
#include "../layout/resource/rom.h"
#include "ty_dp_define.h"
#include "../api/network/network_common.h"
#include "../include/api/video_perf_trace.h"

/* ********************************************************************************************************************* */
/* *************************************************涂鸦本地存储相关***************************************************** */
/* ********************************************************************************************************************* */
#define Debug_Lib (printf("\n\033[0;33;40m[***%s***]:%u\033[0m \t", __PRETTY_FUNCTION__, __LINE__), printf)

extern unsigned long long os_get_ms(void);

static sem_t upload_start;
static atomic_int upload_enable = ATOMIC_VAR_INIT(0);
static long int tuya_upload_count = 0;
static video_perf_stats tuya_main_ring_perf_stats;
static video_perf_stats tuya_sub_ring_perf_stats;
static video_perf_stats tuya_ring_perf_stats;
static unsigned long long tuya_ring_perf_last_trace_ms;

void tuya_occupted_upload(void)
{
    extern unsigned char *get_rom_bin_base(void);
    extern unsigned long long os_get_ms(void);
    unsigned char *data = (unsigned char *)(get_rom_bin_base() + ROM_RES_YUV420SP_TO_H264_H264);
    tuya_realtime_video_put_frame(data, ROM_RES_YUV420SP_TO_H264_H264_SIZE, os_get_ms());
}
void tuya_blank_screen_upload(MEDIA_FRAME_TYPE_E type)
{
    extern unsigned char *get_rom_bin_base(void);
    extern unsigned long long os_get_ms(void);
    unsigned char *data = (unsigned char *)(get_rom_bin_base() + ROM_RES_BLANK_SCREEN_H264);
    tuya_realtime_video_put_frame(data, ROM_RES_BLANK_SCREEN_H264_SIZE, os_get_ms());
}

void tuya_stream_storage_start(void)
{
    tuya_ipc_ss_start_event(0);
}

void tuya_stream_storage_stop(bool keep_upload)
{
    int value;
    sem_getvalue(&upload_start, &value);
    Debug_Lib("tuya_ipc_ss_stop_event==================================================>>>>>%d \n", value);
    atomic_store(&upload_enable, 1); /* 这必须放到前面，否则信号量会优先触发，导致循环第一次误判 */
    if (value == 0)
    {
        tuya_ipc_ss_stop_event(0);
        tuya_upload_count = 0;

        if (keep_upload)
            sem_post(&upload_start);
    }
}

bool tuya_stream_storage_recording(void)
{
    int tuya_record_status = tuya_ipc_ss_get_status();
    return (tuya_record_status != E_STORAGE_READY_TO_STOP) && (tuya_record_status != E_STORAGE_STOP);
}

bool tuya_stream_storage_stopped(void)
{
    int tuya_record_status = tuya_ipc_ss_get_status();
    return (tuya_record_status != E_STORAGE_ONGOING) && (tuya_record_status != E_STORAGE_START);
}

void tuya_upload_disable(void)
{
    atomic_store(&upload_enable, 0);
}

void *tuya_stream_storage_thread(void *arg)
{
    Debug_Lib("******************************************START\n");
    int ss_state = -1;
    while (1)
    {
        sem_wait(&upload_start);
        ss_state = tuya_ipc_ss_get_status();
        Debug_Lib("wait stream storage stop finish,curr state :%d,upload_enable:%d\n", ss_state, atomic_load(&upload_enable));
        while (ss_state == E_STORAGE_ONGOING && atomic_load(&upload_enable))
        {
            ss_state = tuya_ipc_ss_get_status();
            // Debug_Lib("wait stream storage stop finish,curr state :%d\n",ss_state);
            tuya_upload_count++;
            MEDIA_FRAME_TYPE_E type = (tuya_upload_count % 5) == 0 ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME;
            tuya_blank_screen_upload(type);
            ak_sleep_ms(40);
            continue;
        }
        tuya_ring_buffer_clear();

        ak_sleep_ms(50);
    }
    ak_thread_exit();
    return NULL;
}

void tuya_stream_storage_init(void)
{
    ak_pthread_t sd_format_thread;
    ak_thread_create(&sd_format_thread, tuya_stream_storage_thread, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(sd_format_thread);
}

/* ********************************************************************************************************************* */
/* *************************************************涂鸦云存储相关***************************************************** */
/* ********************************************************************************************************************* */
static BOOL_T cloud_storage_stop_flag = false;
static void _cloud_storage_stop_wait(void)
{
    if (cloud_storage_stop_flag == true)
    {
        BOOL_T status = tuya_ipc_cloud_storage_get_status();
        if (status)
        {
            tuya_ipc_cloud_storage_event_stop_async();
            if (tuya_client_num_get() <= 0)
            {
                tuya_blank_screen_upload(E_VIDEO_I_FRAME);
            }
        }
        else
        {
            cloud_storage_stop_flag = false;
            if (tuya_client_num_get() <= 0)
            {
                tuya_ring_buffer_clear();
            }
        }
        db_log_debug("tuya_ipc_cloud_storage_get_status:%d\n", status);
    }
}

int tuya_cloud_storage_start(void)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    db_log_debug("tuya_ipc_cloud_storage_event_start:%d\n", tuya_ipc_cloud_storage_event_start());
    return 0;
}

int tuya_cloud_storage_stop(void)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    if (tuya_ipc_cloud_storage_get_status())
    {
        cloud_storage_stop_flag = true;
        db_log_debug("tuya_ipc_cloud_storage_event_stop_async:%d\n", tuya_ipc_cloud_storage_event_stop_async());
    }
    return 0;
}

/* ********************************************************************************************************************* */
/* ****************************************************涂鸦天气获取****************************************************** */
/* ********************************************************************************************************************* */
static tuya_api_weather tuya_api_werather_const = {0};

static bool _tuya_api_weather_get(tuya_api_weather *nm)
{
    if (tuya_online_status_get() == false)
    {
        return false;
    }

    printf("%s,%d==================>>>>\n\n\n", __func__, __LINE__);
    const char *weather_choose[] = {
        "{                              \
            \"codes\":[                 \
                \"w.humidity\",     \
                \"w.date.1\",           \
                \"w.pressure\"          \
                \"w.pm10\",         \
                \"w.pm25\",        \
                \"w.currdate\",         \
                \"w.temp\",             \
                \"w.conditionNum\",     \
                \"w.thigh\",            \
                \"w.tlow\",             \
                \"c.city\",             \
                ]                       \
        }"

    };

    cJSON *forecast = NULL;
    OPERATE_RET ret = tuya_http_gw_ipc_custom_msg("tuya.device.public.data.get", "1.0", *weather_choose, &forecast);

    if (forecast == NULL)
    {
        printf("%s,%d forecast is NULL\n", __func__, __LINE__);
        return false;
    }
    if (ret)
    {
        printf("get weather fail\n");
        return false;
    }

    cJSON *data_json = cJSON_GetArrayItem(forecast, 0);
    if (data_json == NULL)
    {
        printf("%s,%d data_json is NULL\n", __func__, __LINE__);
        return false;
    }
    // printf("%s  \n", cJSON_Print(cJSON_Parse(cJSON_Print(data_json))));

    /* 用字符串来解析json， 防止返回的数据发生变化时， 解析错误 */
    cJSON *wHumidity = cJSON_GetObjectItem(data_json, "w.humidity");
    cJSON *wPressure = cJSON_GetObjectItem(data_json, "w.pressure");
    cJSON *wPm10 = cJSON_GetObjectItem(data_json, "w.pm10");
    cJSON *wPm25 = cJSON_GetObjectItem(data_json, "w.pm25");
    cJSON *wTemp = cJSON_GetObjectItem(data_json, "w.temp");
    cJSON *wCond = cJSON_GetObjectItem(data_json, "w.conditionNum");
    cJSON *wThigh = cJSON_GetObjectItem(data_json, "w.thigh.0");
    cJSON *wTlow = cJSON_GetObjectItem(data_json, "w.tlow.0");

    /* 数据不为空时， 才能使用 */
    if (wHumidity != NULL)
        sscanf(cJSON_Print(wHumidity), "%d", &(nm->humidity));

    if (wPressure != NULL)
        sscanf(cJSON_Print(wPressure), "%d", &(nm->pressure));

    if (wPm10 != NULL)
        sscanf(cJSON_Print(wPm10), "%d", &(nm->pm10));

    if (wPm25 != NULL)
        sscanf(cJSON_Print(wPm25), "%d", &(nm->pm25));

    if (wTemp != NULL)
        sscanf(cJSON_Print(wTemp), "%d", &(nm->temp));

    if (wCond != NULL)
        sscanf(cJSON_Print(wCond), "\"%d\"", &(nm->condition));

    if (wThigh != NULL)
        sscanf(cJSON_Print(wThigh), "%d", &(nm->thigh));

    if (wTlow != NULL)
        sscanf(cJSON_Print(wTlow), "%d", &(nm->tlow));

    char *formatted = cJSON_Print(forecast);
    if (formatted != NULL)
    {
        printf("%s", formatted);
        free(formatted);
    }

    cJSON_Delete(forecast);
    printf("%s,%d==================>>>>\n\n\n", __func__, __LINE__);
    return true;
}

tuya_api_weather *tuya_weather_get(void)
{
    return &tuya_api_werather_const;
}

/* ********************************************************************************************************************* */
/* ****************************************************涂鸦接口封装****************************************************** */
/* ********************************************************************************************************************* */
extern RING_BUFFER_USER_HANDLE_T s_ring_buffer_handles[E_IPC_STREAM_MAX];
extern CHAR_T s_app_version[64];
int tuya_realtime_video_put_frame(unsigned char *data, int size, unsigned long long pts)
{
    static unsigned long long last_trace_pts;
    static unsigned long long last_missing_handle_trace_pts;
    static unsigned long frame_count;
    int main_ret;
    int sub_ret;
    MEDIA_FRAME_TYPE_E frame_type;
    unsigned long long main_start_ms;
    unsigned long long sub_start_ms;
    unsigned long long now_ms;

    if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] && s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB])
    {
        frame_type = tuya_h264_frame_type_get(data, size);
        main_start_ms = os_get_ms();
        main_ret = tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN], data, size, frame_type, pts);
        sub_start_ms = os_get_ms();
        sub_ret = tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB], data, size, frame_type, pts);
        now_ms = os_get_ms();
        video_perf_stats_record(&tuya_main_ring_perf_stats, size, sub_start_ms - main_start_ms,
                                main_ret, false, 0, 15);
        video_perf_stats_record(&tuya_sub_ring_perf_stats, size, now_ms - sub_start_ms,
                                sub_ret, false, 0, 15);
        video_perf_stats_record(&tuya_ring_perf_stats, size * 2, now_ms - main_start_ms,
                                main_ret != 0 ? main_ret : sub_ret, false, 0, 15);
        frame_count++;

        if ((last_trace_pts == 0) || (pts - last_trace_pts >= 5000))
        {
            Debug_Lib("[TUYA_VIDEO_TRACE] ring append: count=%lu size=%d nal=%u frame_type=%d main_ret=%d sub_ret=%d pts=%llu\n",
                      frame_count, size, data[4] & 0x1f, frame_type, main_ret, sub_ret, pts);
            last_trace_pts = pts;
        }

        if ((tuya_ring_perf_last_trace_ms == 0) ||
            (now_ms - tuya_ring_perf_last_trace_ms >= 5000))
        {
            Debug_Lib("[TUYA_PERF_TRACE] tuya-ring: total calls=%lu bytes=%llu avg=%llums max=%llums slow=%lu fail=%lu | main avg=%llums max=%llums slow=%lu fail=%lu | sub avg=%llums max=%llums slow=%lu fail=%lu\n",
                      tuya_ring_perf_stats.calls, tuya_ring_perf_stats.bytes,
                      tuya_ring_perf_stats.calls ? tuya_ring_perf_stats.total_ms / tuya_ring_perf_stats.calls : 0,
                      tuya_ring_perf_stats.max_ms, tuya_ring_perf_stats.slow_calls, tuya_ring_perf_stats.failures,
                      tuya_main_ring_perf_stats.calls ? tuya_main_ring_perf_stats.total_ms / tuya_main_ring_perf_stats.calls : 0,
                      tuya_main_ring_perf_stats.max_ms, tuya_main_ring_perf_stats.slow_calls, tuya_main_ring_perf_stats.failures,
                      tuya_sub_ring_perf_stats.calls ? tuya_sub_ring_perf_stats.total_ms / tuya_sub_ring_perf_stats.calls : 0,
                      tuya_sub_ring_perf_stats.max_ms, tuya_sub_ring_perf_stats.slow_calls, tuya_sub_ring_perf_stats.failures);
            video_perf_stats_reset(&tuya_main_ring_perf_stats);
            video_perf_stats_reset(&tuya_sub_ring_perf_stats);
            video_perf_stats_reset(&tuya_ring_perf_stats);
            tuya_ring_perf_last_trace_ms = now_ms;
        }

        return main_ret != 0 ? main_ret : sub_ret;
    }

    if ((last_missing_handle_trace_pts == 0) || (pts - last_missing_handle_trace_pts >= 5000))
    {
        Debug_Lib("[TUYA_VIDEO_TRACE] ring append skipped: main=%p sub=%p size=%d pts=%llu\n",
                  (void *)s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN],
                  (void *)s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB], size, pts);
        last_missing_handle_trace_pts = pts;
    }
    return -1;
}

int tuya_realtime_audio_put_frame(unsigned char *data, int size, unsigned long long pts)
{
    if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN])
    {
        tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN], data, size, E_AUDIO_FRAME, pts);
        return 0;
    }
    return -1;
}

void tuya_ring_buffer_clear(void)
{
    tuya_ipc_ring_buffer_clean_user_state_and_buffer(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN]);
    tuya_ipc_ring_buffer_clean_user_state_and_buffer(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB]);
    tuya_ipc_ring_buffer_clean_user_state_and_buffer(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN]);
    tuya_ipc_ring_buffer_get_frame(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN], true);
    tuya_ipc_ring_buffer_get_frame(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB], true);
    tuya_ipc_ring_buffer_get_frame(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN], true);
}

int tuya_notify_call_event(int ch, const uint8_t *jpeg_buf, int size)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    tuya_ipc_notify_door_bell_press(jpeg_buf, size, NOTIFICATION_CONTENT_JPEG);
    tuya_ipc_dp_report(NULL, TUYA_DP_DOOR_BELL, PROP_STR, "tuya6", 1);
    tuya_ipc_notify_alarm(jpeg_buf, size, NOTIFICATION_NAME_DOORBELL, TRUE, NULL);
    return 0;
}

int tuya_notify_alarm_event(int ch, const uint8_t *jpeg_buf, int size)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    tuya_ipc_notify_alarm(jpeg_buf, size, ch == 0 ? NOTIFICATION_NAME_IO_ALARM : NOTIFICATION_NAME_USER_IO, TRUE, NULL);
    return 0;
}

int tuya_notify_motion_event(int ch, const uint8_t *jpeg_buf, int size)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    tuya_ipc_notify_alarm(jpeg_buf, size, NOTIFICATION_NAME_MOTION, TRUE, NULL);
    return 0;
}

bool tuya_net_time_sync(struct tm *new_tm)
{
    if (tuya_online_status_get() == false)
    {
        return -1;
    }
    unsigned int time_t;
    int time_tone;
    if (tuya_ipc_get_service_time(&time_t, &time_tone) == 0)
    {
        struct tm old_tm;
        tuya_ipc_get_local_time(time_t, new_tm);
        return true;
    }
    return false;
}

void tuya_device_unbinding(void)
{
    http_gw_reset(2);
}

int tuya_sub_version_report(int channel, const char *version)
{
    GW_ATTACH_ATTR_T attach_arr[1];
    if (version)
    {
        attach_arr->tp = DEV_ATTACH_MOD_1 + channel - 1;
        strcpy(attach_arr->ver, version);
    }
    else
    {
        attach_arr->tp = DEV_NM_ATH_SNGL;
        strcpy(attach_arr->ver, s_app_version);
    }
    return tuya_iot_dev_update_attachs(CNTSOF(attach_arr), attach_arr);
}

/* ********************************************************************************************************************* */
/* *************************************************涂鸦初始化相关***************************************************** */
/* ********************************************************************************************************************* */
typedef struct
{
    tuya_init_config_t cfg;
    int runing;
    pthread_t tid;
} tuya_private_data;

static tuya_private_data tuya_ctx = {
    .runing = 0,
};

static char tuya_network_online_check(const char *dev)
{
    char on_line = 0x00;

    char network_state[128] = {0};
    sprintf(network_state, "/sys/class/net/%s/operstate", dev);
    int fd = open(network_state, O_RDONLY);

    if (fd < 0)
    {
        return on_line;
    }

    char buffer[128] = {0};
    read(fd, buffer, 2);
    close(fd);

    if (strncmp(buffer, "down", 4) == 0)
    {
        return on_line;
    }

    on_line = 0x01;

#define PING_WWW "www.microsoft.com"

    struct hostent *url = gethostbyname(PING_WWW);

    if (url != NULL)
    {
        printf("official hostname:%s\n", url->h_name);
        on_line = 0x02;
    }

    return on_line;
}

static void *tuya_sdk_thread(void *arg)
{
    tuya_private_data *d = (tuya_private_data *)arg;
    tuya_api_weather tmp_weather = {0};
    struct ak_timeval tv1, tv2;
    while (tuya_network_online_check(d->cfg.net_dev) != 0x02)
    {
        sleep(2);
    }

    ak_get_ostime(&tv1);

    ak_get_ostime(&tv2);

    ty_sdk_main(&d->cfg);

    tuya_stream_storage_init();

    while (d->runing)
    {
        ak_get_ostime(&tv1);
        if (((tv1.sec - tv2.sec > 60) || (tuya_weather_get()->condition == 0)) && (_tuya_api_weather_get(&tmp_weather) == true))
        {
            tv2 = tv1;
            tuya_api_werather_const = tmp_weather;
            extern void get_network_time(void);
            get_network_time(); // 80ms
        }

        _cloud_storage_stop_wait();

        int num = tuya_ipc_get_client_online_num();
        if (num == 0 && tuya_client_num_get() != num)
        {
            tuya_client_num_reset();
            extern bool tuya_monitor_quit_event(void);
            tuya_monitor_quit_event();
        }

        usleep(1000 * 1000);
    }
    return NULL;
}

int tuya_sdk_init(const tuya_init_config_t *cfg)
{
    if (tuya_ctx.runing)
    {
        db_log_error("tuya context runing \n");
        return -1;
    }
    memcpy(&tuya_ctx.cfg, cfg, sizeof(tuya_init_config_t));
    tuya_ctx.runing = 1;
    pthread_create(&tuya_ctx.tid, NULL, tuya_sdk_thread, &tuya_ctx);
    return 0;
}
