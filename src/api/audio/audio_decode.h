#ifndef _AUDIO_DECODE_H_
#define _AUDIO_DECODE_H_
#include <stdbool.h>



bool audio_decode_open(int vol);

bool audio_decode_start(void);

bool audio_decode_stop(void);

bool audio_decode_close(void);

bool audio_decode_queue_push(unsigned char* data ,int len);

void audio_decode_device_open(int vol,bool force);
#endif

