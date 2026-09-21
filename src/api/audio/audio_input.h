#ifndef _AUDIO_INPUT_H_
#define _AUDIO_INPUT_H_
#include <stdbool.h>

bool audio_input_open(void);

bool audio_input_start(void);

bool audio_input_stop(void);

bool audio_input_close(void);

#endif

