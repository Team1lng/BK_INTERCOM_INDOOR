#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_media_stream_event.h"
#include "ty_sdk_common.h"

#include "tkl_audio.h"
#include "tkl_video_enc.h"
#include "tkl_system.h"
#include "tal_thread.h"

RING_BUFFER_USER_HANDLE_T s_ring_buffer_handles[E_IPC_STREAM_MAX] = {
    NULL,
};

void * __video_main(void *args)
{
    UCHAR_T *frame_buff = NULL;
    int bufSize = MAX_MEDIA_FRAME_SIZE;
    int naluSeiLen = 0;
    TKL_VENC_FRAME_T frame = {0};

    frame_buff = (UCHAR_T *)malloc(bufSize);
    if (NULL == frame_buff) {
        PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_VIDEO_MAIN);
        return NULL;
    }
    frame.pbuf = (char *)frame_buff;
    frame.buf_size = bufSize;
    while(1){
        if (0 == tkl_venc_get_frame(0, 0, &frame)) {

            if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] == NULL) {
                s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_VIDEO_MAIN, E_RBUF_WRITE);
            }

            if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN]) {
                tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN], frame_buff,
                                                 frame.used_size + naluSeiLen, frame.frametype, frame.pts);

            } else {
                PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_VIDEO_MAIN);
            }
        } else {
            tkl_system_sleep(10);
        }
    }
    free(frame_buff);
    if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN] != NULL) {
        tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_MAIN]);
    }
    return NULL;
}

void * __video_sub(void *args)
{
    UCHAR_T *frame_buff = NULL;
    int bufSize = MAX_MEDIA_FRAME_SIZE;
    int naluSeiLen = 0;
    TKL_VENC_FRAME_T frame = {0};

    frame_buff = (UCHAR_T *)malloc(bufSize);
    if (NULL == frame_buff) {
        PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_VIDEO_SUB);
        return NULL;
    }
    frame.pbuf = (char *)frame_buff;
    frame.buf_size = bufSize;
    while(1){
        if (0 == tkl_venc_get_frame(0, 1, &frame)) {

            if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] == NULL) {
                s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_VIDEO_SUB, E_RBUF_WRITE);
            }

            if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB]) {
                tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB], frame_buff,
                                                 frame.used_size + naluSeiLen, frame.frametype, frame.pts);

            } else {
                PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_VIDEO_SUB);
            }
        } else {
            tkl_system_sleep(10);
        }
    }
    free(frame_buff);
    if (s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB] != NULL) {
        tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_VIDEO_SUB]);
    }
    return NULL;
}

void * __audio_main(void *args)
{
    UCHAR_T *frame_buff = NULL;
    int bufSize = 1280;
    TKL_AUDIO_FRAME_INFO_T frame = {0};
    frame_buff = (UCHAR_T *)malloc(bufSize);
    if (NULL == frame_buff) {
        PR_ERR("malloc failed type = %d\n", E_IPC_STREAM_AUDIO_MAIN);
        return NULL;
    }
    frame.pbuf = (char *)frame_buff;
    frame.buf_size = bufSize;
    while(1){
        if (0 ==  tkl_ai_get_frame(0, 0, &frame)) {
            if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] == NULL) {
                s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] = tuya_ipc_ring_buffer_open(0, 0, E_IPC_STREAM_AUDIO_MAIN, E_RBUF_WRITE);
            }

            if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN]) {
                tuya_ipc_ring_buffer_append_data(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN], frame_buff, frame.used_size,
                                                     frame.type, frame.pts);

            } else {
                PR_ERR("tuya_ipc_ring_buffer_open failed,channle:%d\n", E_IPC_STREAM_AUDIO_MAIN);
            }
        } else {
            tkl_system_sleep(10);
        }
    }
    free(frame_buff);
    if (s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN] != NULL) {
        tuya_ipc_ring_buffer_close(s_ring_buffer_handles[E_IPC_STREAM_AUDIO_MAIN]);
    }
    return NULL;
}


VOID TUYA_IPC_av_start()
{
    printf("create av task!!!\n"); 
    int ret = 0;
    ret = tkl_vi_init(NULL, 1);
    if (0 != ret) {
        PR_ERR("ty_dsp_init failed,%d\n", ret);
        return ;
    }

    ret = tkl_venc_init(0, NULL, 4);
    if (0 != ret) {
        PR_ERR("ty_dsp_init failed,%d\n", ret);
        return ;
    }

    ret = tkl_ai_init(-1, 1);
    if (0 != ret) {
        PR_ERR("tal_ai_init failed,%d\n", ret);
        return ;
    }


    THREAD_CFG_T video_main_thrd_param;
    video_main_thrd_param.priority = THREAD_PRIO_2;
    video_main_thrd_param.stackDepth = 512 * 1024;
    video_main_thrd_param.thrdname = "video_main";
    THREAD_HANDLE video_main_thrd_hdl;
    tal_thread_create_and_start(&video_main_thrd_hdl, NULL, NULL, __video_main, NULL, &video_main_thrd_param);

    THREAD_CFG_T video_sub_thrd_param;
    video_sub_thrd_param.priority = THREAD_PRIO_2;
    video_sub_thrd_param.stackDepth = 512 * 1024;
    video_sub_thrd_param.thrdname = "video_main";
    THREAD_HANDLE video_sub_thrd_hdl;
    tal_thread_create_and_start(&video_sub_thrd_hdl, NULL, NULL, __video_sub, NULL, &video_sub_thrd_param);

    THREAD_CFG_T audio_main_thrd_param;
    audio_main_thrd_param.priority = THREAD_PRIO_2;
    audio_main_thrd_param.stackDepth = 512 * 1024;
    audio_main_thrd_param.thrdname = "audio_main";
    THREAD_HANDLE audio_main_thrd_hdl;
    tal_thread_create_and_start(&audio_main_thrd_hdl, NULL, NULL, __audio_main, NULL, &audio_main_thrd_param);

    return ; 
}
