/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-04 10:33:17
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-12-05 09:29:21
 * @FilePath: /two-wire-indoor/src/api/audio/audio_vqe.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "audio_vqe.h"
#include "ak_common_audio.h"
#if 0
struct ak_audio_nr_attr user_ai_nr_attr = {USER_AI_NOISE_SUP_DB, 0, ENABLE};
struct ak_audio_agc_attr user_ai_agc_attr = {USER_AI_AGC_LEVEL, USER_AI_AGC_MAX_GAIN, USER_AI_AGC_MIN_GAIN, USER_AI_NEAR_SENSITIVE, 0, ENABLE};
struct ak_audio_aec_attr user_ai_aec_attr = {0, USER_AEC_OUT_DIGI_GAIN, USER_AEC_IN_DIGI_GAIN, 0, USER_AEC_TAIL, ENABLE, 9830};
struct ak_audio_aslc_attr user_ai_aslc_attr = {USER_AI_LIMIT, USER_AI_DB, ENABLE};

/* set ao voice quality enhancement param */
struct ak_audio_nr_attr user_ao_nr_attr = {USER_AO_NOISE_SUP_DB, 0, ENABLE};
struct ak_audio_aslc_attr user_ao_aslc_attr = {USER_AO_LIMIT, USER_AO_DB, ENABLE};
/* gain */

int user_ai_gain = 4;

/*			### how to set ai gain ###
ak_ai_set_gain(ai_handle_id,user_ai_gain);*/

int user_ao_gain = 4;

/*			### how to set ao gain ###
ak_ao_set_gain(ao_handle_id,user_ao_gain);*/

struct ak_audio_eq_attr user_ai_eq_attr = {
    0,

    10,

    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},

    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},

    0,

    0,

    0,

    0,

    0,

    0,

    0,

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

};

struct ak_audio_eq_attr user_ao_eq_attr = {
    0,

    10,

    {50, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    {717, 717, 717, 717, 717, 717, 717, 717, 717, 717},

    {TYPE_HPF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},

    0,

    0,

    0,

    0,

    0,

    0,

    0,

    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

};
#endif