/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-02-20 15:07:08
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-03-30 10:02:55
 * @FilePath: /two-wire-indoor/src/api/common/leo_audio_play.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _LEO_AUDIO_PLAY_H_
#define _LEO_AUDIO_PLAY_H_
#include "stdbool.h"
void touch_sound_play(int volume);
void alarm_sound_play(int volume);
void chime_sound_play(int ring_index, int volume, audio_play_callback start, audio_play_callback end);

bool door_ring_play(int ring_index, int volume, bool send_net, audio_play_callback start, audio_play_callback end);

bool custom_music_play(char *music_name, int volume, bool send_net, audio_play_callback start, audio_play_callback end);
bool interphone_ring_play(int volume, bool send_net, audio_play_callback start, audio_play_callback end);

bool open_door_ring_play(int volume);
bool audio_volume_set(int vol);
bool audio_play_volume_set(int vol);

#endif
