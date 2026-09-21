#ifndef _VIDEO_DECODE_H_
#define _VIDEO_DECODE_H_
#include "stdbool.h"
#include "ak_common_graphics.h"
#include"leo_api.h"

#define DECODE_WIDTH 1920 // 1024
#define DECODE_HIGHT 1080 // 600


void video_decode_init(void);

bool video_decode_open(char, int src_width, int src_height);

bool video_decode_close(void);

bool get_video_decode_state(void);

bool video_decode_push(char, unsigned char *data, int len);

bool video_decode_queue_reset(void);

void video_raw_color_rect(int color,int w,int h,int pos_x,int pos_w);

void video_raw_clear(void);

bool video_decode_resolution_contrast(int src_width, int src_height);

void video_raw_init(void);

bool video_raw_lcd_push(unsigned char *addres, unsigned long phy, int width, int height, int pixel_width, int pixel_height, AK_GP_FORMAT format);

// bool video_raw_push(unsigned char* addres,unsigned long phy,int width,int height);

unsigned char *video_raw_lcd_get(unsigned long long *timestamp);

void video_raw_release_all(void);

bool jpg_record(const char *file_path, char record_mode);

bool is_jpg_record_ing(void);

void video_record_init(void);

bool video_record_start(const char *path, bool has_audio, int width, int height, char type,char record_mode,int media_type, void (*finish_callback)(const char *path));

bool video_record_stop(void);

bool is_video_recording(void);

void wait_video_record_finish(void);

bool video_record_data_push(record_data_node *node);

void video_play_init(void);

bool send_tuya_record(char record_mode);

void jpg_push_to_tuya(int type);

bool video_decode_data_status(void);

bool frame_skip_enable_status(void);
#endif
