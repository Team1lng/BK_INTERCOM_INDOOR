#ifndef LEO_FILE_LIST_H
#define LEO_FILE_LIST_H
#include "stdbool.h"

#define MEDIA_PATH_MAX 128

#define SD_MIXED_MAX 500
#define SD_PHOTO_MAX 100
#define SD_VIDEO_MAX 50
#define SD_MUSIC_MAX 20000
#define FLASH_PHOTO_MAX 15
#define SD_PICTURE_MAX 20000
#define SD_RECORD_MAX 20000

#define FILE_NAME_MAX 24

#define SD_BASE_PATH0 "/mnt/tf0"
#define SD_BASE_PATH "/mnt/tf"
#define SD_BASE_PATH2 "/mnt/tf2"

#define SD_MIXED_PATH SD_BASE_PATH "/media/"
#define SD_PHOTO_PATH SD_BASE_PATH "/photo/"
#define SD_VIDEO_PATH SD_BASE_PATH "/video/"
#define SD_MUSIC_PATH SD_BASE_PATH "/music/"
#define SD_PICTURE_PATH SD_BASE_PATH "/picture/"
#define FLASH_PHOTO_PATH APP_DEFAULT_PATH "photo/"
#define SD_BACKUP_PATH SD_BASE_PATH "/backup/"

#define SD_CALL_PATH SD_BASE_PATH "/call_record/"
#define SD_MSG_PATH SD_BASE_PATH "/msg_record/"
#define SD_MOTION_PATH SD_BASE_PATH "/motion_record/"
#define SD_ALARM_PATH SD_BASE_PATH "/alarm_record/"

#define MIX_PHOTOS_AND_VIDEOS_FILE (0)
#define BG_MIUSIC_FILE_ENABLE (1)
#define BG_PICTURE_FILE_ENABLE (1)
#define REC_CALL_FILE_ENABLE (1)
#define REC_MSG_FILE_ENABLE (1)
#define REC_MOTION_FILE_ENABLE (1)
#define REC_ALARM_FILE_ENABLE (1)
typedef enum
{

    RECORD_MODE_MANUAL = 0,
    RECORD_MODE_AUTO
} record_mode;

typedef enum
{

    PHOTO_TYPE = 0,
    VIDEO_TYPE,
    AUDIO_TYPE,
    TEXT_TYPE
} record_file_type;

typedef enum
{
    FILE_TYPE_SD_MIXED,
    FILE_TYPE_SD_MIXED_PHOTO,
    FILE_TYPE_SD_MIXED_VIDEO,
    FILE_TYPE_SD_PHOTO,
    FILE_TYPE_SD_VIDEO,
    FILE_TYPE_SD_MUSIC,
    FILE_TYPE_SD_PICTURE,
    FILE_TYPE_SD_CALL,
    FILE_TYPE_SD_MSG,
    FILE_TYPE_SD_MOTION,
    FILE_TYPE_SD_ALARM,
    FILE_TYPE_FLASH_PHOTO,
    FILE_TYPE_NONE
} media_type;

enum delete_flag
{
    DELETE_FINISH_STOP = 0X00,
    DELETE_ALL_MIXED = 0X01,
    DELETE_ALL_SD_PHOTO = 0X02,
    DELETE_ALL_SD_VIDEO = 0X04,
    DELETE_ALL_MIXED_PHOTO = 0X08,
    DELETE_ALL_MIXED_VIDEO = 0X10,
    DELETE_ALL_FLASH_PHOTO = 0X20,
    DELETE_ALL_MUSIC = 0X40,
    DELETE_ALL_PICTURE = 0X80,
    DELETE_ALL_CALL = 0X100,
    DELETE_ALL_MSG = 0X200,
    DELETE_ALL_MOTION = 0X400,
    DELETE_ALL_ALARM = 0X800
};

typedef struct
{
    char file_name[FILE_NAME_MAX];
    char mode;
    char ch;
    char is_new;
    media_type type;
    char is_lock;
    record_file_type file_type;
} media_info;

void media_file_list_init(void);
void forced_scan_media_file(media_type type);
bool is_sdcard_insert(void);
bool create_one_media_file(media_type type, char ch, char mode, record_file_type file_type, char *file);
media_info *media_info_get(media_type type, int index);
int media_file_total_get(media_type type, char is_new);
int media_file_new_clear(media_type type, int index);
int media_file_delete(media_type type, int index);
int media_bad_path_check(const char *file, char mode);
bool Media_bad_path_check(const char *file, int index);

void start_format_sd_card(int type);
/* @return: true: formatting false:finish or stop*/
bool format_sd_card_status(void);

void start_copy_flash_photo_to_sd(void);
/* @return: true: copying false:finish or stop*/
bool copy_flash_photo_to_sd_status(void);

void start_delete_media(enum delete_flag item);
/* @return: >0: delete false:finish or stop*/
int delete_media_status(void);

int media_file_lock_set(media_type type, int index, bool en);
/***********************************************
** 作者: leo.liu
** 日期: 2022-11-5 17:11:35
** 说明: 查看sd卡信息
***********************************************/
bool tuya_sd_memory_query(unsigned int *p_total, unsigned int *p_user, unsigned int *p_free);

bool _sd_card_repartition_main(void);
#endif
