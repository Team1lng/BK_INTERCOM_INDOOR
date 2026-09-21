/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-09-22 08:54:21
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-26 08:23:13
 * @FilePath: /two-wire-indoor/src/api/wlan/tuya_stream_storage.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "tuya_stream_storage.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ring_buffer.h"
#include "ak_thread.h"
#include "ak_common.h"
#include "stdbool.h"
#include <stdatomic.h>
#include "../../layout/resource/rom.h"

#define Debug_Lib (printf("\n\033[0;33;40m[***%s***]:%u\033[0m \t", __PRETTY_FUNCTION__, __LINE__), printf)

static sem_t upload_start;
static atomic_int upload_enable = ATOMIC_VAR_INIT(0);
static long int tuya_upload_count = 0;

void tuya_occupted_upload(void)
{
    extern unsigned char *get_rom_bin_base(void);
    extern unsigned long long os_get_ms(void);
    unsigned char *data = (unsigned char *)(get_rom_bin_base() + ROM_RES_YUV420SP_TO_H264_H264);
    tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, ROM_RES_YUV420SP_TO_H264_H264_SIZE, E_VIDEO_I_FRAME, os_get_ms());
}
void tuya_blank_screen_upload(MEDIA_FRAME_TYPE_E type)
{
    extern unsigned char *get_rom_bin_base(void);
    extern unsigned long long os_get_ms(void);
    unsigned char *data = (unsigned char *)(get_rom_bin_base() + ROM_RES_BLANK_SCREEN_H264);
    tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN, data, ROM_RES_BLANK_SCREEN_H264_SIZE, type, os_get_ms());
}

void tuya_stream_storage_stop(bool keep_upload)
{
    int value;
    sem_getvalue(&upload_start, &value);
    Debug_Lib("tuya_ipc_ss_stop_event==================================================>>>>>%d \n", value);
    atomic_store(&upload_enable, 1); /* 这必须放到前面，否则信号量会优先触发，导致循环第一次误判 */
    if (value == 0)
    {
        tuya_ipc_ss_stop_event();
        tuya_upload_count = 0;

        if (keep_upload)
            sem_post(&upload_start);
    }
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
        extern void tuya_ipc_ring_buffer_video_release_data(void);
        tuya_ipc_ring_buffer_video_release_data();

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