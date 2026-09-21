#include "tuya_sdk.h"
#include "wifi_hwl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include "tuya_iot_config.h"
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "base_hwl.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "base_hwl.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_api.h"
#include "network_common.h"
#include "../../include/g711/g711_table.h"
#include "ak_mem.h"
#include "tuya_g711_utils.h"
#include "ak_thread.h"
// #include "os_sys_api.h"

// #include "leo_monitor.h"

OPERATE_RET hwl_bnw_get_mac(OUT NW_MAC_S *mac)
{
    return 1;
}

OPERATE_RET hwl_bnw_set_mac(IN CONST NW_MAC_S *mac)
{
    return 1;
}

OPERATE_RET tuya_iot_reg_wf_lock_chan_cb(FUNC_WIFI_LOCK_CHANNEL_CB func_wifi_lock_ch_cb)
{
    printf("================%s:%d =========== \n", __func__, __LINE__);
    return 1;
}

OPERATE_RET wf_nw_set_lock_chan_notify_cb(FUNC_WIFI_LOCK_CHANNEL_CB func_wifi_lock_ch_cb)
{
    printf("================%s:%d =========== \n", __func__, __LINE__);
    return 1;
}

// #include "audio_package.h"
#include "wlan.h"

#define WLAN_DEV "wlan0"
#define WIRED_DEV "eth2"

#define WLAN_DEVICE false
#define WIRED_DEVICE true
static user_net_pairing *network_dev = NULL;

/***
**   日期:2022-06-17 17:51:32
**   作者: leo.liu
**   函数作用：ping 网址
**   参数说明:
***/
#include <fcntl.h>
char tuya_network_online_check(void)
{
    user_net_pairing pair = WLAN_DEVICE;
    if (network_dev == NULL)
    {
        network_dev = &pair;
    }
    char on_line = 0x00;
#define DETECT_WAN0_PTAH "/sys/class/net/wlan0/operstate"
#define DETECT_ETH2_PTAH "/sys/class/net/eth2/operstate"
    int fd = open(*network_dev == WLAN_DEVICE ? DETECT_WAN0_PTAH : DETECT_ETH2_PTAH, O_RDONLY);
    if (fd < 0)
    {
        printf("open %s fail\n", *network_dev == WLAN_DEVICE ? DETECT_WAN0_PTAH : DETECT_ETH2_PTAH);
        return on_line;
    }
    char buffer[128] = {0};
    read(fd, buffer, 2);
    close(fd);

    if (strncmp(buffer, "down", 4) == 0)
    {
        printf("strncmp %s fail buffer:%s\n", *network_dev == WLAN_DEVICE ? DETECT_WAN0_PTAH : DETECT_ETH2_PTAH, buffer);
        return on_line;
    }
    on_line = 0x01;

#define PING_WWW "www.microsoft.com"
    FILE *pfd = popen("ping -W 2 -c 2 -i 0.9 " PING_WWW, "r");
    if (pfd == NULL)
    {
        printf("open ping " PING_WWW " fail \n");
        return on_line;
    }

    while (fgets(buffer, sizeof(buffer), pfd))
    {
        //	printf("%s \n",buffer);
        char *pstr = strstr(buffer, "ttl=");
        if (pstr != NULL)
        {
            on_line = 0x02;
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }
    pclose(pfd);
    return on_line;
}

void tuya_network_dev_set(user_net_pairing *net_pairing)
{
    network_dev = net_pairing;
}

OPERATE_RET hwl_bnw_get_ip(OUT NW_IP_S *ip)
{
    int sock;
    struct sockaddr_in *sin;
    struct ifreq ifr;
    // linked_info wlan = {0};
    // get_linked_wifi_info(&wlan);
    // if(!wlan.completed){
    //     return OPRT_COM_ERROR;
    // }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket create failse...GetLocalIp!\n");
        return OPRT_COM_ERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    // printf("network_dev ===>>>%d!\n",*network_dev);
    strncpy(ifr.ifr_name, *network_dev ? WIRED_DEV : WLAN_DEV, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
    {
        printf("%s:%d ioctl error\n", __func__, __LINE__);
        close(sock);
        return OPRT_COM_ERROR;
    }

    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    strcpy(ip->ip, inet_ntoa(sin->sin_addr));
    close(sock);
    return OPRT_OK;
}

BOOL_T hwl_bnw_station_conn(VOID)
{
    int sock;
    struct ifreq ifr;
    // linked_info wlan = {0};
    // get_linked_wifi_info(&wlan);
    // if(!wlan.completed){
    //     return OPRT_COM_ERROR;
    // }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("socket create failse...GetLocalIp!\n");
        return OPRT_COM_ERROR;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, *network_dev ? WIRED_DEV : WLAN_DEV, sizeof(ifr.ifr_name) - 1);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
    {
        printf("%s:%d ioctl error\n", __func__, __LINE__);
        close(sock);
        return FALSE;
    }
    close(sock);

    if (0 == (ifr.ifr_flags & IFF_UP))
    {
        return FALSE;
    }

    return TRUE;
}
OPERATE_RET hwl_bnw_set_station_connect(IN CONST CHAR_T *ssid, IN CONST CHAR_T *passwd)
{
    return OPRT_COM_ERROR;
}
BOOL_T hwl_bnw_need_wifi_cfg(VOID)
{
    return FALSE;
}
OPERATE_RET hwl_bnw_station_get_conn_ap_rssi(OUT SCHAR_T *rssi)
{
    *rssi = 99;

    return OPRT_OK;
}

static void p2p_media_init(IPC_MEDIA_INFO_S *info)
{
    memset(info, 0, sizeof(IPC_MEDIA_INFO_S));
    info->channel_enable[E_CHANNEL_VIDEO_MAIN] = TRUE;                 /* Whether to enable local HD video streaming */
    info->video_fps[E_CHANNEL_VIDEO_MAIN] = 25;                        /* FPS */
    info->video_gop[E_CHANNEL_VIDEO_MAIN] = 30;                        /* GOP */
    info->video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_2M; /* Rate limit */
    info->video_width[E_CHANNEL_VIDEO_MAIN] = 1920;                    /* Single frame resolution of width*/
    info->video_height[E_CHANNEL_VIDEO_MAIN] = 1080;                   /* Single frame resolution of height */
    info->video_freq[E_CHANNEL_VIDEO_MAIN] = 90000;                    /* Clock frequency */
    info->video_codec[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H264;   /* Encoding format */
    tuya_ipc_ring_buffer_init(E_CHANNEL_VIDEO_MAIN, info->video_bitrate[E_CHANNEL_VIDEO_MAIN], info->video_fps[E_CHANNEL_VIDEO_MAIN], 0, NULL);

    /* Audio stream configuration.
    Note: The internal P2P preview, cloud storage, and local storage of the SDK are all use E_CHANNEL_AUDIO data. */
    info->channel_enable[E_CHANNEL_AUDIO] = TRUE;                   /* Whether to enable local sound collection */
    info->audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_PCM;      // TUYA_CODEC_AUDIO_PCM; //TUYA_CODEC_AUDIO_PCM /* Encoding format */
    info->audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_16K;    // TUYA_AUDIO_SAMPLE_8K /* Sampling Rate */
    info->audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_16; /* Bit width */
    info->audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_MONO; // TUYA_AUDIO_CHANNEL_MONO;/* channel */
    info->audio_fps[E_CHANNEL_AUDIO] = 32;                          /* Fragments per second */
    tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO, info->audio_sample[E_CHANNEL_AUDIO] * info->audio_databits[E_CHANNEL_AUDIO] / 1024, info->audio_fps[E_CHANNEL_AUDIO], 0, NULL);
}

void p2p_ringbuffer_init(void)
{

    IPC_MEDIA_INFO_S info;
    memset(&info, 0, sizeof(IPC_MEDIA_INFO_S));
    p2p_media_init(&info);
    //     tuya_ipc_ring_buffer_init(E_CHANNEL_VIDEO_MAIN, info.video_bitrate[E_CHANNEL_VIDEO_MAIN],info.video_fps[E_CHANNEL_VIDEO_MAIN],0,NULL);
    //    tuya_ipc_ring_buffer_init(E_CHANNEL_AUDIO,  info.audio_sample[E_CHANNEL_AUDIO]*info.audio_databits[E_CHANNEL_AUDIO]/1024 , info.audio_fps[E_CHANNEL_AUDIO],0,NULL);
}

static void p2p_status_change_func(TRANSFER_ONLINE_E stat)
{
    printf("%s:%d stat:%d \n", __func__, __LINE__, stat);
}

static void p2p_audio_rev_form_app_func(const TRANSFER_AUDIO_FRAME_S *p_audio_frame, const UINT_T frame_no) // 接受涂鸦APP音频
{
    if (tuya_monitor_state_get())
    {
        return;
    }
    // printf("recv audio size:%d \n",p_audio_frame->buf_len);

    // Local play

    /*
    unsigned char pcm_data[640] = {0};
    unsigned int pcm_len = 0;
    tuya_g711_decode(TUYA_G711_MU_LAW, (unsigned short*)p_audio_frame->p_audio_buf,p_audio_frame->buf_len, pcm_data, &pcm_len);
    audio_decode_api.write((char*)pcm_data, pcm_len);
    */
    //	if(audio_package_api.pull_to_local){
    //		audio_package_api.send_write(p_audio_frame->p_audio_buf,p_audio_frame->buf_len);
    //	}
    static char *alaw_buffer = NULL;
    static int read_size = 0;
#define ALAW_BUFFER_MAX 512
    if (alaw_buffer == NULL)
    {
        alaw_buffer = ak_mem_alloc(MODULE_ID_APP, ALAW_BUFFER_MAX);
        read_size = 0;
    }

    if (read_size + (p_audio_frame->buf_len / 2) > ALAW_BUFFER_MAX)
    {
        network_audio_send_package_push(0, alaw_buffer, read_size, false);
        read_size = 0;
    }
    int encode_size = 0;
    tuya_g711_encode(TUYA_G711_A_LAW, (short unsigned int *)p_audio_frame->p_audio_buf, p_audio_frame->buf_len, (unsigned char *)&alaw_buffer[read_size], (unsigned int *)&encode_size);
    read_size += encode_size;
    // pcm16_to_alaw(p_audio_frame->buf_len,(const char *)p_audio_frame->p_audio_buf,alaw_buffer);

    // unsigned long long os_get_ms(void);
    // static unsigned long long frist_count_ms = 0;
    // static int count = 0;
    // count ++;
    // if(count  == 100)
    // {
    // 	count = 0;
    // 	// printf("push end ms =================>>>%lld\n\r",os_get_ms());
    // 	printf("p2p_audio_rev_form_app_func  ms =================>>>%lld\n\r",(os_get_ms() - frist_count_ms)/100);
    // 	frist_count_ms = os_get_ms();
    // }

    // network_audio_send_package_push(0,(const char *)p_audio_frame->p_audio_buf,p_audio_frame->buf_len);
    // audio_decode_queue_push((unsigned char*)p_audio_frame->p_audio_buf,p_audio_frame->buf_len);
}

OPERATE_RET ipc_app_sync_utc_time(VOID)
{
    TIME_T time_utc;
    INT_T time_zone;
    printf("Get Server Time \n");
    OPERATE_RET ret = tuya_ipc_get_service_time_force(&time_utc, &time_zone);

    if (ret != OPRT_OK)
    {
        return ret;
    }
    return OPRT_OK;
}
void tuya_ipc_ring_buffer_video_release_data(void)
{
    // return;
    printf(" %s============================>>>%d\n\r", __func__, __LINE__);
    extern VOID tuya_ipc_ring_buffer_clean_user_state(CHANNEL_E channel, USER_INDEX_E userIndex);
    extern VOID tuya_ipc_ring_buffer_clean_user_state_and_buffer(CHANNEL_E channel, USER_INDEX_E userIndex);
    tuya_ipc_ring_buffer_clean_user_state(E_CHANNEL_VIDEO_MAIN, E_USER_P2P_USER);
    tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_VIDEO_MAIN, E_USER_P2P_USER);
    tuya_ipc_ring_buffer_clean_user_state(E_CHANNEL_AUDIO, E_USER_P2P_USER);
    tuya_ipc_ring_buffer_clean_user_state_and_buffer(E_CHANNEL_AUDIO, E_USER_P2P_USER);
    printf(" %s============================>>>%d\n\r", __func__, __LINE__);
}

OPERATE_RET http_gw_ipc_custom_msg(IN CONST CHAR_T *api_name, IN CONST CHAR_T *api_version, IN CONST CHAR_T *message, OUT cJSON **result);
bool tuya_api_weather_get(void)
{
    printf("================================%s\n\r", __func__);
    cJSON *result = NULL;
    if (http_gw_ipc_custom_msg("tuya.device.public.data.get", "1.0", "{\"codes\":[\"w.humidity\", \"w.temp\"]}", &result))
    {
        printf("http_gw_ipc_custom_msg =====================+>fail\n");
        return false;
    }

    cJSON *data_json = cJSON_GetArrayItem(result, 0);
    cJSON *condition_json = cJSON_GetArrayItem(data_json, 0);
    cJSON *temp_json = cJSON_GetArrayItem(data_json, 1);
    printf("data_json================================%p\n\r", data_json->next);
    printf("result->next================================%p\n\r", temp_json);
    printf("cJSON_GetArraySize================================%d\n\r", cJSON_GetArraySize(result));
    printf("type:%d  valuedouble:%f string:%s   valueint:%d\n", data_json->type, data_json->valuedouble, data_json->string, data_json->valueint);
    printf("type:%d  valuedouble:%f string:%s   valueint:%d\n", condition_json->type, condition_json->valuedouble, condition_json->string, condition_json->valueint);
    printf("type:%d  valuedouble:%f string:%s   valueint:%d\n", temp_json->type, temp_json->valuedouble, temp_json->string, temp_json->valueint);
    cJSON_Delete(result);
    return true;
}

/************************************************************************
 *
 * 录像相关函数
 *
 * ***********************************************************************/
#include "tuya_ipc_stream_storage.h"
#include "file_api.h"
/***********************************************
** 作者: leo.liu
** 日期: 2022-11-5 17:6:55
** 说明: 获取sd 卡状态
***********************************************/
E_SD_STATUS tuya_ipc_sd_get_status(VOID)
{
    return is_sdcard_insert() ? SD_STATUS_NORMAL : SD_STATUS_NOT_EXIST;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-5 17:9:30
** 说明: 获取sd挂载路径
***********************************************/
CHAR_T *tuya_ipc_get_sd_mount_path(VOID)
{
    return SD_BASE_PATH;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-5 17:10:35
** 说明: 获取sd卡使用信息
***********************************************/
VOID tuya_ipc_sd_get_capacity(UINT_T *p_total, UINT_T *p_used, UINT_T *p_free)
{
    bool reslut = false;

    *p_total = *p_used = *p_free = 0;

#ifndef SDCARD_PARTITION
    UINT_T total, used, free;
    reslut = tuya_sd_memory_query(&total, &used, &free);
    if (reslut == false)
    {
        return;
    }

    UINT_T reserved_size;
    reserved_size = total * 1.0 / 3;  // 取三分之一作預留
    int temp = free - reserved_size;  // 计算当前剩余与预留的差
    *p_total = total - reserved_size; // 留给涂鸦的总容量大小
    *p_free = temp < 0 ? 0 : temp;    // 当前若剩余空间小于预留空间，即返回涂鸦剩余空间0
    *p_used = (*p_total) - (*p_free); // 当前涂鸦使用容量大小
    printf("============= tuya sd disk:total:%0.2fM/ free:%0.2fM  user size:%0.fM =============\n", (*p_total) / (1024.0), (*p_free) / (1024.0), (*p_used) / 1024.0);
#else
    reslut = tuya_sd_memory_query(p_total, p_used, p_free);

    if (reslut == false)
    {
        return;
    }

#endif
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-7 13:47:19
** 说明: 涂鸦记录写入模式
***********************************************/
STREAM_STORAGE_WRITE_MODE_E tuya_ipc_sd_get_mode_config(void)
{
    // printf("=====%s:%d =======\n",__func__,__LINE__);
    return SS_WRITE_MODE_EVENT;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-7 13:48:17
** 说明: 移除sdcard 挂载
***********************************************/
void tuya_ipc_sd_remount(void)
{
    printf("=====%s:%d =======\n", __func__, __LINE__);
}

extern VOID IPC_APP_report_sd_format_status(INT_T status);
extern VOID handle_DP_SD_STORAGE_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp);
STATIC INT_T s_sd_format_progress = 0;
INT_T IPC_APP_get_sd_format_status(VOID)
{
    return s_sd_format_progress;
}
void *thread_sd_format(void *arg)
{
    /* First notify to app, progress 0% */

    s_sd_format_progress = 0;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    ak_sleep_ms(100);

    /* Stop local SD card recording and playback, progress 10%*/
    s_sd_format_progress = 10;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
    tuya_ipc_ss_pb_stop_all();
    ak_sleep_ms(100);

    /* Delete the media files in the SD card, the progress is 30% */
    s_sd_format_progress = 30;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    // tuya_ipc_ss_delete_all_files();
    ak_sleep_ms(100);

    /* Perform SD card formatting operation */
    start_format_sd_card(2);
    while (format_sd_card_status() && is_sdcard_insert())
    {
        ak_sleep_ms(100);
    }
    s_sd_format_progress = 80;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    // TODO
    //     IPC_APP_set_sd_record_onoff( IPC_APP_get_sd_record_onoff());

    /* progress 100% */
    ak_sleep_ms(100);
    s_sd_format_progress = 100;
    IPC_APP_report_sd_format_status(s_sd_format_progress);

    tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);

    // handle_DP_SD_STORAGE_ONLY_GET(NULL);
    printf("format sd_card finish!!!!\r\n");

    ak_thread_exit();
    return NULL;
}
VOID IPC_APP_format_sd_card(VOID)
{
    printf("start to format sd_card \r\n");
    /* SD card formatting.
     * The SDK has already completed the writing of some of the code,
     and the developer only needs to implement the formatting operation. */

    ak_pthread_t sd_format_thread;
    ak_thread_create(&sd_format_thread, thread_sd_format, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
    ak_thread_detach(sd_format_thread);
}

/***************************************录像case*****************************************************/
typedef struct
{
    BOOL_T enabled;
    TRANSFER_VIDEO_CLARITY_TYPE_E live_clarity;
    UINT_T max_users;
    TUYA_CODEC_ID p2p_audio_codec;
} TUYA_APP_P2P_MGR;

// STATIC TUYA_APP_P2P_MGR s_p2p_mgr = {0};

STATIC VOID __TUYA_APP_media_frame_TO_trans_video(IN CONST MEDIA_FRAME_S *p_in, INOUT TRANSFER_VIDEO_FRAME_S *p_out)
{
    UINT_T codec_type = 0;
    codec_type = (p_in->type & 0xff00) >> 8;
    p_out->video_codec = (codec_type == 0 ? TUYA_CODEC_VIDEO_H264 : TUYA_CODEC_VIDEO_H265);
    p_out->video_frame_type = (p_in->type && 0xff) == E_VIDEO_PB_FRAME ? TY_VIDEO_FRAME_PBFRAME : TY_VIDEO_FRAME_IFRAME;
    p_out->p_video_buf = p_in->p_buf;
    p_out->buf_len = p_in->size;
    p_out->pts = p_in->pts;
    p_out->timestamp = p_in->timestamp;
}

STATIC VOID __TUYA_APP_media_frame_TO_trans_audio(IN CONST MEDIA_FRAME_S *p_in, INOUT TRANSFER_AUDIO_FRAME_S *p_out)
{
    p_out->audio_codec = TUYA_CODEC_AUDIO_PCM;
    p_out->audio_sample = TUYA_AUDIO_SAMPLE_16K;
    p_out->audio_databits = TUYA_AUDIO_DATABITS_16;
    p_out->audio_channel = TUYA_AUDIO_CHANNEL_MONO;
    p_out->p_audio_buf = p_in->p_buf;
    p_out->buf_len = p_in->size;
    p_out->pts = p_in->pts;
    p_out->timestamp = p_in->timestamp;
}

STATIC VOID __TUYA_APP_ss_pb_event_cb(IN UINT_T pb_idx, IN SS_PB_EVENT_E pb_event, IN PVOID_T args)
{
    printf("ss pb rev event: %u %d", pb_idx, pb_event);
    if (pb_event == SS_PB_FINISH)
    {
        tuya_ipc_playback_send_finish(pb_idx);
    }
}

STATIC VOID __TUYA_APP_ss_pb_get_video_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_S *p_frame)
{
    TRANSFER_VIDEO_FRAME_S video_frame = {0};
    __TUYA_APP_media_frame_TO_trans_video(p_frame, &video_frame);
    tuya_ipc_playback_send_video_frame(pb_idx, &video_frame);
}

STATIC VOID __TUYA_APP_ss_pb_get_audio_cb(IN UINT_T pb_idx, IN CONST MEDIA_FRAME_S *p_frame)
{
    TRANSFER_AUDIO_FRAME_S audio_frame = {0};
    __TUYA_APP_media_frame_TO_trans_audio(p_frame, &audio_frame);
    tuya_ipc_playback_send_audio_frame(pb_idx, &audio_frame);
}
/*********************************************结束**********************************************************************/

static TRANSFER_EVENT_E tuya_event_state = TRANS_LIVE_AUDIO_STOP;
TRANSFER_EVENT_E tuya_event_state_get(void)
{
    return tuya_event_state;
}

TRANSFER_EVENT_E tuya_event_state_set(TRANSFER_EVENT_E state)
{
    tuya_event_state = state;
    return tuya_event_state;
}

#include "tuya_ipc_mqt_proccess.h"

// VOID tuya_ipc_mqt_rtc_callback(IN CONST IPC_RTC_MQTT_DATA_CB pcbk)
// {
//     printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
//     printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
//     printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
//     printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
//     printf("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
// }

#include "leo_api.h"
INT_T tuya_online_clinet_num = 0;
INT_T tuya_online_clinet_num_get(void)
{
    return tuya_online_clinet_num;
}
void tuya_online_clinet_num_reset(void)
{
    tuya_online_clinet_num = 0;
}

static INT_T p2p_event_func(const TRANSFER_EVENT_E event, const PVOID_T args)
{
    // return 0;

    printf("%s=============>>>%d \n", __func__, event);
    switch (event)
    {
    case TRANS_LIVE_VIDEO_START: // 涂鸦获取视频
    {
        tuya_online_clinet_num++;
        // int on_line = tuya_ipc_get_client_online_num();
        if (tuya_event_state == TRANS_LIVE_VIDEO_START)
        {
            break;
        }
        tuya_event_state = TRANS_LIVE_VIDEO_START;
        ipc_app_sync_utc_time();

        /* 这里屏蔽 tuya_ipc_ring_buffer_video_release_data是因为，如果在tuya_ipc_ss_start_event开启过程中调用，会导致tuya_ipc_ss_stop_event时出现关闭延时，在开启事件期间不要清除ring_buffer*/
        tuya_ipc_ring_buffer_video_release_data();

        extern bool tuya_monitor_enter_event(void);
        tuya_monitor_enter_event();
        printf("%s=============>>> tuya video start \n", __func__);
    }
    break;

    case TRANS_LIVE_VIDEO_STOP: // tuya退出视频
    {
        tuya_online_clinet_num--;
        // int on_line = tuya_ipc_get_client_online_num();
        if (tuya_online_clinet_num > 0)
        {
            break;
        }
        // printf("=============>>> tuya video stop %d\n",tuya_ipc_get_client_online_num());
        tuya_event_state = TRANS_LIVE_VIDEO_STOP;
        extern bool tuya_monitor_quit_event(void);
        tuya_monitor_quit_event();
    }
    break;

    case TRANS_LIVE_AUDIO_START: //
    {
        int on_line = tuya_ipc_get_client_online_num();
        if (on_line > 1 || tuya_monitor_state_get())
        {
            break;
        }
        //			audio_push_to_tuya_open();
        printf("=============>>> tuya audio start \n");
    }
    break;

    case TRANS_LIVE_AUDIO_STOP:
    {
        int on_line = tuya_ipc_get_client_online_num();
        printf("=============>>> tuya audio stop %d\n", tuya_ipc_get_client_online_num());
        if (on_line > 1)
        {
            break;
        }
        //			audio_push_to_tuya_close();
        printf("=============>>> tuya audio stop \n");
    }
    break;

    case TRANS_SPEAKER_START: // 传输到涂鸦的
    {
        // extern int tuya_current_channel_get(void);
        // static bool on_line = tuya_ipc_get_client_online_num();
        // if(on_line > 1 || tuya_monitor_state_get()/*  || tuya_current_channel_get() */)
        // {
        // 	break;
        // }
        // amp_enable(true);
        // audio_pull_to_local_open();
        extern bool tuya_monitor_talk_event(bool state);
        tuya_monitor_talk_event(true);
        printf("=============>>> tuya speaker start \n");
    }
    break;

    case TRANS_SPEAKER_STOP:
    {

        // int on_line = tuya_ipc_get_client_online_num();
        // if(on_line > 1)
        // {
        // 	break;
        // }
        // amp_enable(false);
        //			audio_pull_to_local_close();
        extern bool tuya_monitor_talk_event(bool state);
        tuya_monitor_talk_event(false);
        printf("=============>>> tuya speaker stop \n");
    }
    break;
    case TRANS_PLAYBACK_QUERY_MONTH_SIMPLIFY:
    {
        C2C_TRANS_QUERY_PB_MONTH_REQ *p = (C2C_TRANS_QUERY_PB_MONTH_REQ *)args;
        printf("pb query by month: %d-%d", p->year, p->month);

        OPERATE_RET ret = tuya_ipc_pb_query_by_month(p->year, p->month, &(p->day));
        if (OPRT_OK != ret)
        {
            printf("pb query by month: %d-%d ret:%d", p->year, p->month, ret);
        }

        break;
    }
    case TRANS_PLAYBACK_QUERY_DAY_TS:
    {
        C2C_TRANS_QUERY_PB_DAY_RESP *pquery = (C2C_TRANS_QUERY_PB_DAY_RESP *)args;
        printf("pb_ts query by day: idx[%d]%d-%d-%d", pquery->channel, pquery->year, pquery->month, pquery->day);
        SS_QUERY_DAY_TS_ARR_S *p_day_ts = NULL;
        OPERATE_RET ret = tuya_ipc_pb_query_by_day(pquery->channel, pquery->year, pquery->month, pquery->day, &p_day_ts);
        if (OPRT_OK != ret)
        {
            printf("pb_ts query by day: %d-%d-%d-%d Fail", pquery->channel, pquery->year, pquery->month, pquery->day);
            break;
        }
        if (p_day_ts)
        {
            printf("%s %d count = %d\n", __FUNCTION__, __LINE__, p_day_ts->file_count);
            PLAY_BACK_ALARM_INFO_ARR *pResult = (PLAY_BACK_ALARM_INFO_ARR *)malloc(sizeof(PLAY_BACK_ALARM_INFO_ARR) + p_day_ts->file_count * sizeof(PLAY_BACK_ALARM_FRAGMENT));
            if (NULL == pResult)
            {
                printf("%s %d malloc failed \n", __FUNCTION__, __LINE__);
                free(p_day_ts);
                pquery->alarm_arr = NULL;
                break;
            }

            INT_T i;
            pResult->file_count = p_day_ts->file_count;
            for (i = 0; i < p_day_ts->file_count; i++)
            {
                pResult->file_arr[i].type = p_day_ts->file_arr[i].type;
                pResult->file_arr[i].time_sect.start_timestamp = p_day_ts->file_arr[i].start_timestamp;
                pResult->file_arr[i].time_sect.end_timestamp = p_day_ts->file_arr[i].end_timestamp;
            }
            pquery->alarm_arr = pResult;
            free(p_day_ts);
        }
        else
        {
            pquery->alarm_arr = NULL;
        }
        break;
    }
    case TRANS_PLAYBACK_START_TS:
    {
        /* Client will bring the start time when playback.
        For the sake of simplicity, only log printing is done. */
        C2C_TRANS_CTRL_PB_START *pParam = (C2C_TRANS_CTRL_PB_START *)args;
        printf("PB StartTS idx:%d %u [%u %u]", pParam->channel, pParam->playTime, pParam->time_sect.start_timestamp, pParam->time_sect.end_timestamp);

        SS_FILE_TIME_TS_S pb_file_info;
        int ret;
        memset(&pb_file_info, 0x00, sizeof(SS_FILE_TIME_TS_S));
        // memcpy(&pb_file_info, &pParam->time_sect, sizeof(SS_FILE_TIME_TS_S));
        pb_file_info.start_timestamp = pParam->time_sect.start_timestamp;
        pb_file_info.end_timestamp = pParam->time_sect.end_timestamp;
        ret = tuya_ipc_ss_pb_start(pParam->channel, __TUYA_APP_ss_pb_event_cb, __TUYA_APP_ss_pb_get_video_cb, __TUYA_APP_ss_pb_get_audio_cb);
        if (0 != ret)
        {
            printf("%s %d pb_start failed\n", __FUNCTION__, __LINE__);
            tuya_ipc_playback_send_finish(pParam->channel);
        }
        else
        {
            if (0 != tuya_ipc_ss_pb_seek(pParam->channel, &pb_file_info, pParam->playTime))
            {
                printf("%s %d pb_seek failed\n", __FUNCTION__, __LINE__);
                tuya_ipc_playback_send_finish(pParam->channel);
            }
        }

        break;
    }
    case TRANS_PLAYBACK_PAUSE:
    {
        C2C_TRANS_CTRL_PB_PAUSE *pParam = (C2C_TRANS_CTRL_PB_PAUSE *)args;
        printf("PB Pause idx:%d", pParam->channel);

        tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_PAUSE);
        break;
    }
    case TRANS_PLAYBACK_RESUME:
    {
        C2C_TRANS_CTRL_PB_RESUME *pParam = (C2C_TRANS_CTRL_PB_RESUME *)args;
        printf("PB Resume idx:%d", pParam->channel);

        tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_RESUME);
        break;
    }
    case TRANS_PLAYBACK_MUTE:
    {
        C2C_TRANS_CTRL_PB_MUTE *pParam = (C2C_TRANS_CTRL_PB_MUTE *)args;
        printf("PB idx:%d mute", pParam->channel);

        tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_MUTE);
        break;
    }
    case TRANS_PLAYBACK_UNMUTE:
    {
        C2C_TRANS_CTRL_PB_UNMUTE *pParam = (C2C_TRANS_CTRL_PB_UNMUTE *)args;
        printf("PB idx:%d unmute", pParam->channel);

        tuya_ipc_ss_pb_set_status(pParam->channel, SS_PB_UN_MUTE);
        break;
    }
    case TRANS_PLAYBACK_STOP:
    {
        C2C_TRANS_CTRL_PB_STOP *pParam = (C2C_TRANS_CTRL_PB_STOP *)args;
        printf("PB Stop idx:%d", pParam->channel);

        tuya_ipc_ss_pb_stop(pParam->channel);
        break;
    }
    default:
        printf("%s:%d recv evnet:%d \n", __func__, __LINE__, event);
        break;
    }

    printf("%s =============>>>>%d : online:%d :%d\n", __func__, event, tuya_ipc_get_client_online_num(), tuya_online_clinet_num);
    return TRANS_EVENT_SUCCESS;
}

OPERATE_RET TUYA_APP_Enable_P2PTransfer(IN UINT_T max_users)
{
    TUYA_IPC_TRANSFER_VAR_S p2p_var = {0};

    p2p_var.online_cb = p2p_status_change_func;
    p2p_var.on_rev_audio_cb = p2p_audio_rev_form_app_func;

    p2p_var.rev_audio_codec = TUYA_CODEC_AUDIO_PCM;
    p2p_var.audio_sample = TUYA_AUDIO_SAMPLE_16K;
    p2p_var.audio_databits = TUYA_AUDIO_DATABITS_16;
    p2p_var.audio_channel = TUYA_AUDIO_CHANNEL_MONO;
    /*end*/
    p2p_var.on_event_cb = p2p_event_func;
    p2p_var.live_quality = TRANS_LIVE_QUALITY_MAX; // TRANS_LIVE_QUALITY_MAX;
    p2p_var.max_client_num = max_users;
    alaw_pcm16_tableinit();
    p2p_media_init(&p2p_var.AVInfo);
    tuya_ipc_tranfser_init(&p2p_var);
    // iot_register_extra_mqt_cb();
#if 0
    unsigned char receive_frame_buffer[1024] = {0,0,0,1,0x67,0,0,0,1,0x68,0,0,0,1,0x65};
    tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN,
														 (unsigned char *)receive_frame_buffer,
														 1024,
														  E_VIDEO_I_FRAME ,
														 os_get_ms());
    tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO,
														 (unsigned char *)receive_frame_buffer,
														 1024,
														  E_CHANNEL_AUDIO ,
														 os_get_ms());
#endif
    return OPRT_OK;
}

bool tuya_net_time_sync(struct tm *local_time)
{
    TIME_T time_utc;
    INT_T time_zone;
    OPERATE_RET ret = tuya_ipc_get_service_time_force(&time_utc, &time_zone);
    ret += tuya_ipc_get_local_time(time_utc, local_time);
    // DEBUG_LOG("\n\r Now Server Time: %d-%d-%d  %d:%d:%d \n\r",
    //        local_time->tm_year,
    //        local_time->tm_mon,
    //        local_time->tm_mday,
    //        local_time->tm_hour,
    //        local_time->tm_min,
    //        local_time->tm_sec);
    if (ret != OPRT_OK)
    {
        return false;
    }
    return true;
}
IPC_REGISTER_STATUS tuya_get_app_register_status(void)
{
    IPC_REGISTER_STATUS status = tuya_ipc_get_register_status();
    // printf("current register status %d[0:unregistered 1:registered 2:activated]\n",status);
    return status;
}
