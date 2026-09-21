#include "audio_play.h"
#include "leo_audio_play.h"
#include "stdbool.h"
#include "../../lvgl/src/lv_core/lv_obj.h"

extern unsigned long long os_get_ms(void);
extern void audio_play(audio_info *);
bool is_talking = false;

#define RING_PATH RING_FILE_PATH
#define MUSIC_PATH MUSIC_FILE_PATH

void touch_sound_play(int volume)
{

	// audio_play_stop_set();
	if (is_talking)
		return;

	static char *file_info = {RING_PATH "touch.mp3"};
	// printf("%s=====================>>>file_info:%s\n\r",__func__,file_info);

	audio_info info =
		{
			.data_type = 1,
			.data.file_path = file_info,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
			.volume = volume * 5 + 46,
			.interrupt = true,
			.start = NULL,
			.end = NULL};
	audio_play(&info);
}

void chime_sound_play(int ring_index, int volume, audio_play_callback start, audio_play_callback end)
{
	printf("%s,%d\n\n\n", __func__, __LINE__);
	if (is_talking)
		return;

	static char *file_info[] =
		{
			RING_PATH "1.mp3",
			RING_PATH "2.mp3",
			RING_PATH "3.mp3",
			RING_PATH "4.mp3",
			RING_PATH "5.mp3",
			// RING_PATH"10.mp3",
			// RING_PATH"11.mp3",
			// RING_PATH"12.mp3"
		};
	if (ring_index > (sizeof(file_info) / sizeof(char *)) || ring_index == 0)
	{
		return;
	}
	printf("%s ===================>>%s\n", __func__, file_info[ring_index - 1]);
	audio_info info =
		{
			.data_type = 1,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
		};

	info.data.file_path = file_info[ring_index - 1];
	info.volume = volume * 5 + 46; //?volume*5+30:0;
	info.start = start;
	info.end = end;
	info.callback_force = true;
	audio_play(&info);
}

void alarm_sound_play(int volume)
{
	if (is_talking)
		return;

	static char *file_info = {RING_PATH "alarms.mp3"};
	printf("%s=====================>>>file_info:%s\n\r", __func__, file_info);

	audio_info info =
		{
			.data_type = 1,
			.data.file_path = file_info,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
			.volume = volume * 5 + 46,
			.interrupt = false,
			.start = NULL,
			.end = NULL};
	audio_play(&info);
}

bool door_ring_play(int ring_index, int volume, bool send_net, audio_play_callback start, audio_play_callback end)
{
	printf("%s ===================>>%d\n", __func__, ring_index);
	static char *file_info[] =
		{
			RING_PATH "1.mp3",
			RING_PATH "2.mp3",
			RING_PATH "3.mp3",
			RING_PATH "4.mp3",
			RING_PATH "5.mp3",
			// RING_PATH"10.mp3",
			// RING_PATH"11.mp3",
			// RING_PATH"12.mp3"
		};
	if (ring_index > (sizeof(file_info) / sizeof(char *)) || ring_index == 0)
	{
		return false;
	}
	printf("%s,%d,%d\n\n\n", __func__, __LINE__, ring_index);
	printf("%s ===================>>%s\n", __func__, file_info[ring_index - 1]);
	audio_info info =
		{
			.data_type = 1,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
		};

	info.data.file_path = file_info[ring_index - 1];
	info.volume = volume * 5 + 46; //?volume*5+30:0;
	info.start = start;
	info.end = end;
	info.send_net = send_net;
	audio_play(&info);
	return true;
}

bool custom_music_play(char *music_name, int volume, bool send_net, audio_play_callback start, audio_play_callback end)
{
	extern bool is_sdcard_insert(void);
	if (is_sdcard_insert() == false)
		return false;
	printf("%s=====================>>>send_net:%d\n\r", __func__, send_net);
	if (music_name == NULL)
	{
		return false;
	}
	static char music[64] = {0};
	bzero(music, sizeof(music));
	sprintf(music, MUSIC_PATH "%s", music_name);
	printf("music %s=====================>>>play\n\r", music);
	audio_info info =
		{
			.data_type = 1,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
		};
	info.data.file_path = music;
	info.volume = volume * 5 + 46; //?volume*5+30:0;
	info.start = start;
	info.end = end;
	info.send_net = send_net;
	audio_play(&info);
	return true;
}

bool interphone_ring_play(int volume, bool send_net, audio_play_callback start, audio_play_callback end)
{
	printf("%s =============>>>%d\n", __func__, __LINE__);
	audio_info info =
		{
			.data_type = 1,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
		};
	static char *file = RING_PATH "1.mp3";
	info.data.file_path = file;
	info.volume = volume * 5 + 46;
	info.start = start;
	info.end = end;
	info.send_net = send_net;
	audio_play(&info);
	return true;
}

bool open_door_ring_play(int volume)
{
	audio_info info =
		{
			.data_type = 1,
			.ch = AUDIO_CHANNEL_MONO,
			.rate = AK_AUDIO_SAMPLE_RATE_16000,
			.type = AK_AUDIO_TYPE_MP3, //,
		};
	static char *file = RING_PATH "7.mp3";
	info.data.file_path = file;
	info.volume = volume * 5 + 46;
	info.start = NULL;
	info.end = NULL;
	audio_play(&info);
	return true;
}

bool audio_volume_set(int vol)
{
	printf("%s===================>%d\n", __func__, vol);
	return audio_output_volume_set(vol * 5 + 46);
}
bool audio_play_volume_set(int vol)
{
	printf("%s===================>%d\n", __func__, vol);
	return audio_ring_volume_set(vol * 5 + 46);
}
