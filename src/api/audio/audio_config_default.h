#ifndef _AUDIO_CONFIG_DEFAULT_H_
#define _AUDIO_CONFIG_DEFAULT_H_
#ifdef AUDIO_CONFIG_DEFAULT

#define ENABLE (1)
#define DISABLE (0)
/**************************************/
/* user vqe param */
/**************************************/
/* ai agc */
#define USER_AI_AGC_LEVEL (24576) // default (0.75) * (1 << 15)
#define USER_AI_AGC_MIN_GAIN (0)
#define USER_AI_AGC_MAX_GAIN (4)
#define USER_AI_NEAR_SENSITIVE (40)

/*         #### how to set agc ###
struct ak_audio_agc_attr user_ai_agc = {0};
user_ai_agc.agc_level = USER_AI_AGC_LEVEL;
user_ai_agc.agc_max_gain = USER_AI_AGC_MAX_GAIN;
user_ai_agc.agc_min_gain = USER_AI_AGC_MIN_GAIN;
user_ai_agc.near_sensitivity = USER_AI_NEAR_SENSITIVE;
user_ai_agc.enable = ENABLE;
ak_ai_set_agc_attr(ai_handle_id,&user_ai_agc);*/

/* ai nr */
#define USER_AI_NOISE_SUP_DB (-20) // noise suppress db

/*         #### how to set ai nr ###
struct ak_audio_nr_attr user_ai_nr_attr = {0};
user_ai_nr_attr.noise_suppress_db = USER_AI_NOISE_SUP_DB;
user_ai_nr_attr.enable = ENABLE;
ak_ai_set_nr_attr(ai_handle_id,&user_ai_nr_attr);*/

/* ai aec */
#define USER_AEC_OUT_DIGI_GAIN (1024)
#define USER_AEC_IN_DIGI_GAIN (716)
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
#define USER_AI_LIMIT (16384)
#define USER_AI_DB (2)

/*         #### how to set ai aslc ###
struct ak_audio_aslc_attr user_ai_aslc_attr = {0};
user_ai_aslc_attr.aslc_db = USER_AI_DB;
user_ai_aslc_attr.limit = USER_AI_LIMIT;
ak_ai_set_aslc_attr(ai_handle_id, &user_ai_aslc_attr);*/

/* ao nr */
#define USER_AO_NOISE_SUP_DB (-40) // noise suppress db
/*         #### how to set ao nr ###
struct ak_audio_nr_attr user_ao_nr_attr = {0};
user_ao_nr_attr.noise_suppress_db = USER_AO_NOISE_SUP_DB;
user_ao_nr_attr.enable = ENABLE;
ak_ao_set_nr_attr(ao_handle_id,&user_ao_nr_attr);*/

/* ao aslc */
#define USER_AO_LIMIT (9830)
#define USER_AO_DB (10)
/*         #### how to set ao aslc ###
struct ak_audio_aslc_attr user_ao_aslc_attr = {0};
user_ao_aslc_attr.aslc_db = USER_AO_DB;
user_ao_aslc_attr.limit = USER_AO_LIMIT;
ak_ao_set_aslc_attr(ao_handle_id, &user_ao_aslc_attr);*/

/* set ai voice quality enhancement param */
struct ak_audio_nr_attr user_ai_nr_attr;
struct ak_audio_agc_attr user_ai_agc_attr;
struct ak_audio_aec_attr user_ai_aec_attr;
struct ak_audio_aslc_attr user_ai_aslc_attr;

/* set ao voice quality enhancement param */
struct ak_audio_nr_attr user_ao_nr_attr;
struct ak_audio_aslc_attr user_ao_aslc_attr;
/* gain */

int user_ai_gain;

/*			### how to set ai gain ###
ak_ai_set_gain(ai_handle_id,user_ai_gain);*/

int user_ao_gain;

/*			### how to set ao gain ###
ak_ao_set_gain(ao_handle_id,user_ao_gain);*/

struct ak_audio_eq_attr user_ai_eq_attr;

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

struct ak_audio_eq_attr user_ao_eq_attr;
#endif
#endif