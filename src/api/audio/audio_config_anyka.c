#include "audio_config_anyka.h"
#include "ak_common_audio.h"

#ifndef AUDIO_CONFIG_DEFAULT
/* set ai voice quality enhancement param */
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

    1,

    {5000, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},

    {-6144, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    {716, 717, 717, 717, 717, 717, 717, 717, 717, 717},

    {TYPE_HSF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},

    0,

    0,

    0,

    0,

    0,

    0,

    1,

    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0}

};

/*         #### how to set eq ###
struct ak_audio_eq_attr m_eq = {0};
m_eq.pre_gain = (signed short)(0*(1<<10));
//if use exported above attr ,just use like this m_eq.pre_gain = xxx;
m_eq.aslc_ena = 0;
m_eq.aslc_level_max = 0;
m_eq.bands = 1;
m_eq.bandfreqs[0] = 1200;
m_eq.bandgains[0] = (signed short)(0*(1<<10));
//if use exported above attr ,just use like this m_eq.bandgains[0] = xxx;
m_eq.bandQ[0] = (unsigned short)(0.8*(1<<10));
//if use exported above attr ,just use like this m_eq.bandQ[0] = xxx;
m_eq.band_types[0] = TYPE_HPF;
m_eq.enable = 1;
m_eq.band_enable[0] =1;
ak_ai_set_eq_attr(ai_handle, &m_eq);*/

struct ak_audio_eq_attr user_ao_eq_attr = {
    0,

    0,

    {1000, 1400, 5000, 250, 500, 1000, 2000, 4000, 8000, 16000},

    {0, -15360, -15360, 0, 0, 0, 0, 0, 0, 0},

    {716, 1024, 1024, 717, 717, 717, 717, 717, 717, 717},

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