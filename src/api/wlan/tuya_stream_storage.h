#ifndef TUYA_STREAM_STORAGE_H
#define TUYA_STREAM_STORAGE_H
#include"stdbool.h"

void tuya_stream_storage_init(void);
void tuya_occupted_upload(void);
void tuya_stream_storage_stop(bool keep_upload);
#endif