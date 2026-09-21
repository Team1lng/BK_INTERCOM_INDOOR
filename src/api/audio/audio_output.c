#include "audio_output.h"
#include "../../include/g711/g711_table.h"
#include "ak_mem.h"
#ifdef AUDIO_CONFIG_DEFAULT
#include "audio_config_default.h"
#else
#include "audio_config_anyka.h"
#endif
#include "leo_api.h"

static int audio_output_handle_id = -1;

static enum ak_audio_sample_rate auido_output_rate = AK_AUDIO_SAMPLE_RATE_16000;
static enum ak_audio_channel_type audio_ouput_channel = AUDIO_CHANNEL_RESERVED;

static ak_mutex_t audio_output_mutex;
bool audio_output_close(void);
static struct ak_timeval audio_tv;
#if 0
static void setup_default_ao_argument(void *audio_args, char args_type)
{
	// struct ak_audio_nr_attr user_ai_nr_attr = {0};//{-20, 0, 1};//{-30, 0, 1};
	// 	struct ak_audio_agc_attr user_ai_agc_attr = {0};
	// struct ak_audio_aec_attr user_ai_aec_attr = {0};
	// struct ak_audio_aslc_attr user_ai_aslc_attr = {0};

	// struct ak_audio_nr_attr user_ao_nr_attr = {-40, 0, 1};//{-20, 0, 1};//{-30, 0, 1};
	// struct ak_audio_aslc_attr user_ao_aslc_attr = {30000, 5, 0};

	switch (args_type)
	{
	case 1:
		*(struct ak_audio_nr_attr *)audio_args = user_ai_nr_attr;
		break;
	case 2:
		*(struct ak_audio_agc_attr *)audio_args = user_ai_agc_attr;
		break;
	case 3:
		*(struct ak_audio_aec_attr *)audio_args = user_ai_aec_attr;
		break;
	case 4:
		*(struct ak_audio_aslc_attr *)audio_args = user_ai_aslc_attr;
		break;
	case 5:
		*(struct ak_audio_nr_attr *)audio_args = user_ao_nr_attr;
		break;
	case 6:
		*(struct ak_audio_aslc_attr *)audio_args = user_ao_aslc_attr;
		break;
	default:
		break;
	}

	return;
}

#endif

struct ak_timeval audio_output_time_get(void)
{
	return audio_tv;
}

static bool audio_output_mutex_init(void)
{
	static bool is_first_init = true;
	if (is_first_init == true)
	{
		is_first_init = false;
		ak_thread_mutex_init(&audio_output_mutex, NULL);
	}
	return true;
}

static bool audio_output_devices_open(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate)
{
	printf("%s =======================================ch:%d    rate:%d\n\r", __func__, ch, rate);
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	return ak_ao_open(&param, &audio_output_handle_id) ? false : true;
}
static bool audio_output_restart_devices(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate)
{
	printf("%s =======================================ch:%d    rate:%d\n\r", __func__, ch, rate);
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	return ak_ao_reset_params(audio_output_handle_id, &param) ? false : true;
}
static bool audio_output_devices_open_check(void)
{
	return (audio_output_handle_id == -1) ? false : true;
}

static int audio_output_volume = 0;
static int audio_ring_volume = 0;
int audio_ring_volume_get(void)
{
	return audio_ring_volume;
}
bool audio_ring_volume_set(int vol)
{
	static int vol_tmp = 0;
	if (vol_tmp != vol)
	{
		vol_tmp = vol;
		if (vol_tmp >= 100)
		{
			audio_ring_volume = 96;
			vol_tmp = 96;
		}
		else
		{
			audio_ring_volume = vol;
		}
		// printf("audio_ring_volume   : %d\n\r", audio_ring_volume);
	}
	return false;
}
bool audio_output_volume_set(int vol)
{
	static int vol_tmp = 0;
	// printf("%s============================>>%d\n",__func__,audio_output_devices_open_check());
	if (audio_output_devices_open_check() == false)
	{
		audio_output_volume = -1;
		return false;
	}

	if (vol_tmp != vol)
	{
		vol_tmp = vol;
		if (vol_tmp >= 100)
		{
			audio_output_volume = 96;
			vol_tmp = 96;
		}
		else
		{
			audio_output_volume = vol;
		}
		// printf("audio_output_volume   : %d\n\r", audio_output_volume);
		// extern 	unsigned long long os_get_ms(void);
		// printf("====================%llu\n\r",os_get_ms());
		// ak_ao_set_volume(audio_output_handle_id, 10);
		// printf("====================%llu\n\r",os_get_ms());
	}

	// if(audio_output_volume == 70)
	//{
	// ak_ao_set_volume(audio_output_handle_id, audio_output_volume-80);
	//}
	// ak_ao_set_speaker(audio_output_handle_id,1);
	// ak_ao_enable_eq(audio_output_handle_id,0);
	// ak_ao_enable_nr(audio_output_handle_id,0);
	// ak_ao_enable_hs(audio_output_handle_id,0);
	return true;
}

int audio_output_volume_get(void)
{
	return audio_output_volume;
}

enum ak_audio_sample_rate get_curr_audio_sample_rate(void)
{
	return auido_output_rate;
}

void audio_output_restart(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	printf("%s !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", __func__);
	ak_ao_cancel(0);
	ak_ao_restart(0);
	ak_thread_mutex_unlock(&audio_output_mutex);
}

bool audio_output_open(enum ak_audio_channel_type ch, enum ak_audio_sample_rate rate, int vol, int gain)
{
	printf("%s @@  %d  @@  %d  @@  %d  @@  %d  @@@@@@@@@@@@@@@@@@@@@\n", __func__, auido_output_rate, rate, audio_ouput_channel, ch);
	audio_output_mutex_init();

	ak_thread_mutex_lock(&audio_output_mutex);
	if ((audio_output_devices_open_check() == false) || (auido_output_rate != rate) || (audio_ouput_channel != ch))
	{
		if (audio_output_devices_open_check() == true)
		{
			audio_output_restart_devices(ch, rate);
			// audio_output_close();
		}
		else
		{
			audio_output_devices_open(ch, rate);
		}
		auido_output_rate = rate;
		audio_ouput_channel = ch;
		// audio_output_volume = vol;
	}

	// struct ak_audio_nr_attr user_ao_nr_attr = {USER_AO_NOISE_SUP_DB, 0, ENABLE};
	// struct ak_audio_aslc_attr user_ao_aslc_attr = {USER_AO_LIMIT, USER_AO_DB, ENABLE};

	ak_ao_set_nr_attr(audio_output_handle_id, &user_ao_nr_attr);
	ak_ao_set_aslc_attr(audio_output_handle_id, &user_ao_aslc_attr);
	// struct ak_audio_nr_attr nr_attr = {0};
	// setup_default_ao_argument(&nr_attr, 5);
	// ak_ao_set_nr_attr(audio_output_handle_id, &nr_attr);

	// struct ak_audio_aslc_attr aslc_attr;
	// setup_default_ao_argument(&aslc_attr, 6);
	// ak_ao_set_aslc_attr(audio_output_handle_id, &aslc_attr);

	// struct ak_audio_eq_attr user_ao_eq_attr={
	// 	 0,

	// 	1,

	// 	{ 700, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000},

	// 	{0,0,0,0,0,0,0,0,0,0},

	// 	{716,717,717,717,717,717,717,717,717,717},

	// 	{TYPE_HPF,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1,TYPE_PF1},

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	0,

	// 	1,

	// 	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}

	// 	};
	// ak_ao_set_eq_attr(audio_output_handle_id,&user_ao_eq_attr);

	ak_ao_set_gain(audio_output_handle_id, user_ao_gain); // gain);

	ak_ao_set_volume(audio_output_handle_id, USER_AO_DB);
	// int vol1;
	// ak_ao_get_volume(audio_output_handle_id,&vol1);
	// audio_output_volume_set(vol); //(vol);

	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}

#define PCM_SIZE_MAX 4 * 1024

/*
 *	设置16BIT的PCM音量，转换一次的最大size不能超过PCM_SIZE_MAX
 *
 *	src:pcm源数据
 *
 *	size:pcm数据的长度,比如80将音量设置为之前的80%
 *
 */
#if 1
#include "math.h"
static bool pcm_16bit_volume_cover(unsigned char *src, int size, int volume)
{
	static int prev_volume = 0;
	float multiplier = pow(10, (float)(volume - 96) / 20);
	if (prev_volume != volume)
	{
		prev_volume = volume;
		printf("%s===============>>%d - %lf\n", __func__, volume, multiplier);
	}
	// printf("%s===============>>%d - %d - %d\n", __func__, volume, audio_ring_volume, audio_output_volume);
	unsigned char dst[PCM_SIZE_MAX] = {0};

	if (size > PCM_SIZE_MAX)
	{
		return false;
	}

	// if (volume == 0)
	// {
	// 	memset(src, 0, size);
	// 	return true;
	// }

#if 0
	

	float multiplier = pow(10,(volume - 80)/20);
	for (int i = 0; i < 1024; i++) {
		short pcmval = (src[i + 1] << 8) | (src[i] & 0xFF);

		pcmval = src[i] * multiplier;

		if (pcmval < 32767 && pcmval > -32768) {

			src[i] = pcmval;

		} else if (pcmval > 32767) {

			src[i] = 32767;

		} else if (pcmval < -32768) {

			src[i] = -32768;

		}

	}
#else
	memset(dst, 0, PCM_SIZE_MAX);
	for (int i = 0; i < size; i += 2)
	{
		short src_data = (src[i + 1] << 8) | (src[i] & 0xFF);

		src_data = src_data * multiplier;

		if (src_data > 32767)
		{
			printf("src_data :%d\n", src_data);
			src_data = 32767;
		}
		else if (src_data < -32768)
		{
			printf("src_data :%d\n", src_data);
			src_data = -32768;
		}

		dst[i] = src_data & 0xFF;
		dst[i + 1] = (src_data >> 8) & 0xFF;
	}

	memcpy(src, dst, size);
#endif
	return true;
}
#else

int pcm_16bit_volume_cover(unsigned char *src, int size, int volume)
{
	int tmp;
	static int prev_volume = 0;
	if (prev_volume != volume)
	{
		prev_volume = volume;
		printf("%s===============>>%d\n", __func__, volume);
	}
	// in_vol[0, 100]
	float vol = volume - 98;

	if (-98 < vol && vol < 0)
		vol = 1 / (vol * (-1));
	else if (0 <= vol && vol <= 1)
		vol = 1;
	/*
	else if(1<=vol && vol<=2)
		vol = vol;
	*/
	else if (vol <= -98)
		vol = 0;
	else if (vol >= 2)
		vol = 40; // 这个值可以根据你的实际情况去调整

	tmp = (*src) * vol; // 上面所有关于vol的判断，其实都是为了此处*in_buf乘以一个倍数，你可以根据自己的需要去修改

	// 下面的code主要是为了溢出判断
	if (tmp > 32767)
		tmp = 32767;
	else if (tmp < -32768)
		tmp = -32768;
	*src = tmp;

	return 0;
}

#endif
bool network_audio_send_package_push(char type, const char *data, int len, bool ring_back);
/***
**   日期:2022-07-01 09:08:28
**   作者: leo.liu
**   函数作用：将铃声送到网络端
**   参数说明:一个包大小为1510-77= 1433
***/
static void audio_output_cover_send_network(unsigned char *data, int len)
{
	int send_size = 0;
	int remain_size = len;
	// printf("%s =======len:%d\n\r",__func__,len);
	while (remain_size > 1024)
	{
		network_audio_send_package_push(0, (char *)&data[send_size], 1024, true);
		send_size += 1024;
		remain_size -= 1024;
	}
	network_audio_send_package_push(0, (char *)&data[send_size], len - send_size, true);
}

bool pcm_resamplerate(int src_sample, int dst_sample, const char *src_pcm, int src_frames, unsigned char *dst_pcm, int *dst_frames);
static void audio_output_send_network(unsigned char *data, int len, int *temp_fd)
{

#ifdef RING_BACK_MODIFY
	audio_output_cover_send_network((unsigned char *)data, len);
#else
	static unsigned char cover_data[4096] = {0};
	static int main_len = 0;
	int cover_len = 0;
	unsigned char temp[4096];
	char *alaw_buffer = NULL;

	pcm_resamplerate(auido_output_rate, 16000, (const char *)data, len, temp, &cover_len);

	// printf("pcm_resamplerate src_sample:%d\n\r",cover_len);
	if ((main_len + cover_len) > 4096)
	{

		int alaw_buffer_size = main_len / 2;
		alaw_buffer = ak_mem_alloc(MODULE_ID_APP, alaw_buffer_size);
		if (alaw_buffer == NULL)
		{
			printf("Error while allocating memory for alaw_buffer.\n");
			return;
		}

		pcm16_to_alaw(main_len, (const char *)cover_data, alaw_buffer);

		audio_output_cover_send_network((unsigned char *)alaw_buffer, alaw_buffer_size);

		ak_mem_free(alaw_buffer);
		alaw_buffer = NULL;
		main_len = 0;
	}
	memcpy(&cover_data[main_len], temp, cover_len);
	main_len += cover_len;
#endif
}
bool audio_output_write(unsigned char *data, int len, bool send_net, int *temp_fd)
{
	ak_thread_mutex_lock(&audio_output_mutex);

	ak_get_ostime(&audio_tv);
	// ak_ao_print_runtime_status(audio_output_handle_id);
	if (audio_output_devices_open_check() == false)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	int play_len = 0;

	extern bool is_audio_play_ing(void);
	// printf("%s,%d,%d,%d\n", __func__, is_audio_talk_open(), audio_ring_volume, audio_output_volume);
	pcm_16bit_volume_cover(data, len, is_audio_talk_open() != AI_AO_O ? audio_ring_volume : audio_output_volume);
	// int gain, volume;
	// ak_ao_get_gain(audio_output_handle_id, &gain);
	// ak_ao_get_volume(audio_output_handle_id, &volume);
	// printf("audio_output_volume   : %d,gain:%d,volume:%d\n\r", audio_output_volume, gain, volume);

	if (send_net == true)
	{
		audio_output_send_network(data, len, temp_fd);
	}

	// struct ak_dev_buf_status status;
	// unsigned long long cur_pts = os_get_ms();
	// ak_ao_get_buf_status(audio_output_handle_id, &status);
	// printf("buf_remain_len   : %d \n\r", status.buf_remain_len );
	if (ak_ao_send_frame(audio_output_handle_id, data, len, &play_len))
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	// unsigned long long x = os_get_ms();
	// ak_ao_get_buf_status(audio_output_handle_id, &status);
	// printf("GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG:%lld    total:%d len:%d\n",x - cur_pts,status.buf_total_len,status.buf_remain_len);
	// printf("buf_remain_len   : %d,%llums\n\r", status.buf_remain_len,os_get_ms() - cur_pts);
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}

bool audio_output_close(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if (audio_output_handle_id != -1)
	{
		ak_ao_close(audio_output_handle_id);
		audio_output_handle_id = -1;
	}

	auido_output_rate = AK_AUDIO_SAMPLE_RATE_16000;
	audio_ouput_channel = AUDIO_CHANNEL_RESERVED;
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}

bool audio_output_buffer_status_printf(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if (audio_output_handle_id == -1)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}

	struct ak_dev_buf_status status;
	ak_ao_get_buf_status(audio_output_handle_id, &status);
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}

void audio_output_buffer_clear(void)
{
	if (audio_output_handle_id != -1)
		ak_ao_clear_frame_buffer(audio_output_handle_id);
}

bool audio_output_buffer_get(int *total, int *remain)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if (audio_output_handle_id == -1)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}

	struct ak_dev_buf_status status;
	ak_ao_get_buf_status(audio_output_handle_id, &status);
	ak_thread_mutex_unlock(&audio_output_mutex);
	*total = status.buf_total_len;
	*remain = status.buf_remain_len;
	// printf("status.buf_remain_len:%d\n",status.buf_remain_len);
	return true;
}

int ao_get_remain_len(void)
{
	struct ak_dev_buf_status status;
	ak_ao_get_buf_status(audio_output_handle_id, &status);
	return status.buf_remain_len;
}
