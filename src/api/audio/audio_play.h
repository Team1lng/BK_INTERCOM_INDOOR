#ifndef _AUDIO_PLAY_H_
#define _AUDIO_PLAY_H_
#include "audio_output.h"

#include "../../lvgl/src/lv_core/lv_obj.h"
typedef void (*audio_play_callback)(void);



typedef struct
{
	char data_type; //0 rom_bin_info ,1:file path
	union
	{
		rom_bin_info bin;
		char* file_path;
	}data;
	enum ak_audio_channel_type ch;
	enum ak_audio_sample_rate rate;
	enum ak_audio_type type;

	bool send_net;

	bool interrupt;
	
	int volume;

	bool callback_force;
	audio_play_callback start;
	audio_play_callback end;
}audio_info;


bool audio_play_init(void);


bool is_audio_play_ing(void);


bool audio_play_stop_set(void);

#endif

