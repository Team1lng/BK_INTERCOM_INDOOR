/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-04 10:33:17
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-02 15:01:49
 * @FilePath: /two-wire-indoor/src/api/audio/audio_output.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H
#include "ak_ao.h"
#include <stdbool.h>
#include <string.h>

#include "ak_thread.h"

extern ak_mutex_t audio_output_open_mutex;

bool audio_output_open(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate, int vol, int gain);

bool audio_output_write(unsigned char *data, int len, bool send_net, int *temp_fd);

bool audio_output_close(void);

bool audio_output_buffer_status_printf(void);

void audio_output_buffer_clear(void);

bool audio_output_buffer_get(int *total, int *remain);

bool audio_ring_volume_set(int vol);

bool audio_output_volume_set(int vol);

int audio_ring_volume_get(void);

int audio_output_volume_get(void);

struct ak_timeval audio_output_time_get(void);
#endif
