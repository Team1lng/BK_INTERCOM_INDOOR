/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-12-05 08:25:30
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-12-05 09:17:02
 * @FilePath: /two-wire-indoor/src/api/audio/audio_config_anyka.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _AUDIO_CONFIG_ANYKA_H_
#define _AUDIO_CONFIG_ANYKA_H_
#ifndef AUDIO_CONFIG_DEFAULT

#define ENABLE (1)
#define DISABLE (0)
/**************************************/
/* user vqe param */
/**************************************/
/* ai agc */
#define USER_AI_AGC_LEVEL (24576) // default (0.75) * (1 << 15)
#define USER_AI_AGC_MIN_GAIN (0)
#define USER_AI_AGC_MAX_GAIN (4)
#define USER_AI_NEAR_SENSITIVE (80)

/*         #### how to set agc ###      s
struct ak_audio_agc_attr user_ai_agc = {0};
user_ai_agc.agc_level = USER_AI_AGC_LEVEL;
user_ai_agc.agc_max_gain = USER_AI_AGC_MAX_GAIN;
user_ai_agc.agc_min_gain = USER_AI_AGC_MIN_GAIN;
user_ai_agc.near_sensitivity = USER_AI_NEAR_SENSITIVE;
user_ai_agc.enable = ENABLE;
ak_ai_set_agc_attr(ai_handle_id,&user_ai_agc);*/

/* ai nr */
#define USER_AI_NOISE_SUP_DB (-30) // noise suppress db

/*         #### how to set ai nr ###
struct ak_audio_nr_attr user_ai_nr_attr = {0};
user_ai_nr_attr.noise_suppress_db = USER_AI_NOISE_SUP_DB;
user_ai_nr_attr.enable = ENABLE;
ak_ai_set_nr_attr(ai_handle_id,&user_ai_nr_attr);*/

/* ai aec */
#define USER_AEC_OUT_DIGI_GAIN (1024)
#define USER_AEC_IN_DIGI_GAIN (1024)
#define USER_AEC_TAIL (512)

/*         #### how to set aec ###
struct ak_audio_aec_attr user_aec_attr = {0};
user_aec_attr.audio_in_digi_gain = USER_AEC_IN_DIGI_GAIN;
user_aec_attr.audio_out_digi_gain = USER_AEC_OUT_DIGI_GAIN;
user_aec_attr.audio_out_threshold = USER_AEC_OUT_THRESHLD;
user_aec_attr.enable = ENABLE;
user_aec_attr.tail = 512; //don't need to set this value now
ak_ai_set_aec_attr(ai_handle_id, &user_aec_attr);*/

/* ai aslc */
#define USER_AI_LIMIT (32768)
#define USER_AI_DB (6)

/*         #### how to set ai aslc ###
struct ak_audio_aslc_attr user_ai_aslc_attr = {0};
user_ai_aslc_attr.aslc_db = USER_AI_DB;
user_ai_aslc_attr.limit = USER_AI_LIMIT;
ak_ai_set_aslc_attr(ai_handle_id, &user_ai_aslc_attr);*/

/* ao nr */
#define USER_AO_NOISE_SUP_DB (-25) // noise suppress db
/*         #### how to set ao nr ###
struct ak_audio_nr_attr user_ao_nr_attr = {0};
user_ao_nr_attr.noise_suppress_db = USER_AO_NOISE_SUP_DB;
user_ao_nr_attr.enable = ENABLE;
ak_ao_set_nr_attr(ao_handle_id,&user_ao_nr_attr);*/

/* ao aslc */
#define USER_AO_LIMIT (29491)
#define USER_AO_DB (10)
/*         #### how to set ao aslc ###
struct ak_audio_aslc_attr user_ao_aslc_attr = {0};
user_ao_aslc_attr.aslc_db = USER_AO_DB;
user_ao_aslc_attr.limit = USER_AO_LIMIT;
ak_ao_set_aslc_attr(ao_handle_id, &user_ao_aslc_attr);*/

struct ak_audio_nr_attr user_ai_nr_attr;
struct ak_audio_agc_attr user_ai_agc_attr;
struct ak_audio_aec_attr user_ai_aec_attr;
struct ak_audio_aslc_attr user_ai_aslc_attr;

struct ak_audio_nr_attr user_ao_nr_attr;

struct ak_audio_aslc_attr user_ao_aslc_attr;

int user_ai_gain;

int user_ao_gain;

struct ak_audio_eq_attr user_ai_eq_attr;

struct ak_audio_eq_attr user_ao_eq_attr;
#endif
#endif