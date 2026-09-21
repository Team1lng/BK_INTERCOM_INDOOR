#include "audio_input.h"
#include "ak_ai.h"
#include "string.h"
#include "ak_thread.h"
#include "../../include/g711/g711_table.h"
#include "ak_mem.h"
#include "tuya_g711_utils.h"
#include "leo_api.h"

#ifdef AUDIO_CONFIG_DEFAULT
#include "audio_config_default.h"
#else
#include "audio_config_anyka.h"
#endif

bool video_record_data_push(record_data_node *node);
bool network_audio_send_package_push(char type, const char *data, int len, bool ring_back);

static bool audio_input_task_run = false;
static bool audio_input_thread_fun = false;
static bool audio_input_capture = false;

// static void setup_default_audio_argument(void *audio_args, char args_type)
// {
// 	printf("%s============++>>%d\n",__func__,args_type);
// 	// struct ak_audio_nr_attr user_ai_nr_attr = {-40, 0, 1};//{-20, 0, 1};//{-30, 0, 1};
// 	//  struct ak_audio_agc_attr user_ai_agc_attr = {16384/*24576*/, 4, 0, /*80*/10, 0, 1};
// 	 struct ak_audio_aec_attr user_ai_aec_attr = {0, USER_AEC_OUT_DIGI_GAIN, USER_AEC_IN_DIGI_GAIN, 0, USER_AEC_TAIL, ENABLE};
// 	//  struct ak_audio_aslc_attr user_ai_aslc_attr = {23000, 0, 0};//{16384, 0, 0};//{32768, -30, 1};

// 	//  struct ak_audio_nr_attr user_ao_nr_attr = {0};
// 	//  struct ak_audio_aslc_attr user_ao_aslc_attr = {0};
// 	switch (args_type)
// 	{
// 	case 1:
// 		*(struct ak_audio_nr_attr *)audio_args = user_ai_nr_attr;
// 		break;
// 	case 2:
// 		*(struct ak_audio_agc_attr *)audio_args = user_ai_agc_attr;
// 		break;
// 	case 3:
// 		*(struct ak_audio_aec_attr *)audio_args = user_ai_aec_attr;
// 		break;
// 	case 4:
// 		*(struct ak_audio_aslc_attr *)audio_args = user_ai_aslc_attr;
// 		break;
// 	case 5:
// 		*(struct ak_audio_nr_attr *)audio_args = user_ao_nr_attr;
// 		break;
// 	case 6:
// 		*(struct ak_audio_aslc_attr *)audio_args = user_ao_aslc_attr;
// 		break;
// 	default:
// 		break;
// 	}

// 	return;
// }

static int audio_input_device_open(void)
{
	int ai_handle_id = -1;
	struct ak_audio_in_param ai_param;
	ai_param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	ai_param.pcm_data_attr.channel_num = AUDIO_CHANNEL_MONO;
	ai_param.pcm_data_attr.sample_rate = AK_AUDIO_SAMPLE_RATE_16000;
	ai_param.dev_id = DEV_ADC;
	if (ak_ai_open(&ai_param, &ai_handle_id))
	{
		return -1;
	}
	// struct ak_audio_nr_attr user_ai_nr_attr = {USER_AI_NOISE_SUP_DB, 0, ENABLE};
	// struct ak_audio_agc_attr user_ai_agc_attr = {USER_AI_AGC_LEVEL, USER_AI_AGC_MAX_GAIN, USER_AI_AGC_MIN_GAIN, USER_AI_NEAR_SENSITIVE, 0, ENABLE};
	// struct ak_audio_aec_attr user_ai_aec_attr = {0, USER_AEC_OUT_DIGI_GAIN, USER_AEC_IN_DIGI_GAIN, 0, USER_AEC_TAIL, ENABLE, 9830};
	// struct ak_audio_aslc_attr user_ai_aslc_attr = {USER_AI_LIMIT, USER_AI_DB, ENABLE};

	// struct ak_audio_nr_attr nr_attr;
	// setup_default_audio_argument(&nr_attr, 1);
	ak_ai_set_nr_attr(ai_handle_id, &user_ai_nr_attr);
	ak_ai_enable_nr(ai_handle_id, AUDIO_FUNC_ENABLE);

	// struct ak_audio_agc_attr agc_attr;
	// setup_default_audio_argument(&agc_attr, 2);
	ak_ai_set_agc_attr(ai_handle_id, &user_ai_agc_attr);
	ak_ai_enable_agc(ai_handle_id, AUDIO_FUNC_ENABLE);

	// struct ak_audio_aec_attr aec_attr;
	// setup_default_audio_argument(&aec_attr, 3);
	ak_ai_set_aec_attr(ai_handle_id, &user_ai_aec_attr);
	ak_ai_enable_aec(ai_handle_id, AUDIO_FUNC_ENABLE);

	// struct ak_audio_aslc_attr aslc_attr;
	// setup_default_audio_argument(&aslc_attr, 4);
	ak_ai_set_aslc_attr(ai_handle_id, &user_ai_aslc_attr);

	Debug_Lib("user_ai_gain:%d\n", user_ai_gain);
	// struct ak_audio_eq_attr user_ai_eq_attr = {
	// 	0,

	// 	1,

	// 	{5000, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},

	// 	{-6144, 0, 0, 0, 0, 0, 0, 0, 0, 0},

	// 	{716, 717, 717, 717, 717, 717, 717, 717, 717, 717},

	// 	{TYPE_HSF, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1, TYPE_PF1},

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	1,

	// 	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0}

	// };

	ak_ai_set_eq_attr(ai_handle_id, &user_ai_eq_attr);

	ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC);
	ak_ai_set_gain(ai_handle_id, user_ai_gain);

	ak_ai_set_volume(ai_handle_id, USER_AI_DB); // enable_agc

	return ai_handle_id;
}
static void audio_input_device_close(int hand_id)
{
	if (hand_id != -1)
	{
		ak_ai_close(hand_id);
	}
}
static void *audio_input_task(void *arg)
{
	audio_input_thread_fun = true;

	int hand_id = audio_input_device_open();

	ak_ai_start_capture(hand_id);

	ak_sleep_ms(100);
	struct frame pcm_frame = {0};

	char *alaw_buffer = NULL;
	int read_size = 0;
	while (audio_input_task_run == true)
	{
		memset(&pcm_frame, 0, sizeof(struct frame));
		if (ak_ai_get_frame(hand_id, &pcm_frame, 0) == 0)
		{
			if (audio_input_capture == true)
			{
				// video_record_data_push(0, pcm_frame.data,  pcm_frame.len, false);
				// printf("pcm_frame.len ================>%d\n\r",pcm_frame.len);
				// int alaw_buffer_size = pcm_frame.len / 2;
				// alaw_buffer = ak_mem_alloc(MODULE_ID_APP,alaw_buffer_size);
				// if(alaw_buffer == NULL)
				// {
				// 	printf("Error while allocating memory for alaw_buffer.\n");
				// 	continue;
				// }

				// pcm16_to_alaw(pcm_frame.len,(const char *)pcm_frame.data,alaw_buffer);
				// network_audio_send_package_push(0,alaw_buffer, alaw_buffer_size);
				// ak_mem_free(alaw_buffer);
				// alaw_buffer = NULL;

#define ALAW_BUFFER_MAX 512
				if (alaw_buffer == NULL)
				{
					alaw_buffer = ak_mem_alloc(MODULE_ID_APP, ALAW_BUFFER_MAX);
					read_size = 0;
				}

				if (read_size + (pcm_frame.len / 2) > ALAW_BUFFER_MAX)
				{
					network_audio_send_package_push(0, alaw_buffer, read_size, false);
					read_size = 0;
				}
				int encode_size = 0;
				tuya_g711_encode(TUYA_G711_A_LAW, (short unsigned int *)pcm_frame.data, pcm_frame.len, (unsigned char *)&alaw_buffer[read_size], (unsigned int *)&encode_size);
				read_size += encode_size;
			}
			ak_ai_release_frame(hand_id, &pcm_frame);
		}
		ak_sleep_ms(1);
	}
	ak_ai_stop_capture(hand_id);
	audio_input_device_close(hand_id);
	audio_input_thread_fun = false;
	ak_mem_free(alaw_buffer);
	alaw_buffer = NULL;

	printf("%s audio_input_device_close over.............\n\r", __func__);
	ak_thread_exit();
	return NULL;
}

static bool audio_input_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (audio_input_thread_fun == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

bool audio_input_open(void)
{
	if (audio_input_task_run == true)
	{
		return false;
	}

	if (audio_input_wait_thread_quit() == false)
	{
		return false;
	}

	printf("%s ============================>>>\n\r", __func__);
	audio_input_task_run = true;
	audio_input_capture = false;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, audio_input_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool audio_input_start(void)
{
	if (audio_input_capture == true)
	{
		return false;
	}
	audio_input_capture = true;
	return true;
}

bool audio_input_stop(void)
{
	if (audio_input_capture == false)
	{
		return false;
	}
	audio_input_capture = false;
	return true;
}
bool audio_input_close(void)
{
	if (audio_input_task_run == false)
	{
		return false;
	}
	audio_input_task_run = false;
	return true;
}
