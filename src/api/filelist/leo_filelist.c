#include "file_api.h"
#include "ak_mem.h"
#include <dirent.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stdio.h"
#include <sys/vfs.h>

#include <dirent.h>
#include "leo_api.h"
#define UPGRADE_NEW_PACKAGR_PATH "SAT_ANYKAOS_"
#define UPGRADE_OLD_PACKAGR_PATH "two_wire_indoor.update"
#define MNT0_UPGRADE_PACKAGR_PATH "/mnt/tf0/"
#define MNT_UPGRADE_PACKAGR_PATH "/mnt/tf/"
#define TMP_UPGRADE_PACKAGR_PATH "/tmp/"
static void sdcard_upgrade_package_copy(char *src_dir, char *dst_dir, bool rm_backup);

extern bool sdcard_status_change_push(char, char);

static int sd_mixed_total = 0;
static int sd_mixed_new_total = 0;
static media_info *p_sd_mixed;

static int sd_photo_total = 0;
static int sd_photo_new_total = 0;
static media_info *p_sd_photo;

static int sd_video_total = 0;
static int sd_video_new_total = 0;
static media_info *p_sd_video;

static int sd_music_total = 0;
static int sd_music_new_total = 0;
static media_info *p_sd_music;

static int sd_picture_total = 0;
static int sd_picture_new_total = 0;
static media_info *p_sd_picture;

static int flash_photo_total = 0;
static int flash_photo_new_total = 0;
static media_info *p_flash_photo;

static int sd_call_total = 0;
static int sd_call_new_total = 0;
static media_info *p_sd_call;

static int sd_msg_total = 0;
static int sd_msg_new_total = 0;
static media_info *p_sd_msg;

static int sd_motion_total = 0;
static int sd_motion_new_total = 0;
static media_info *p_sd_motion;

static int sd_alarm_total = 0;
static int sd_alarm_new_total = 0;
static media_info *p_sd_alarm;

static char sd_card_inserted = 0;
static ak_mutex_t file_list_mutex;

static struct statfs diskinfo, diskinfo_tuya;

#define VIDEO_DOT ".AVI"
#define PHOTO_DOT ".JPG"
#define PICTURE_DOT ".jpg"
#define PICTURE_E_DOT ".jpeg"
#define MUSIC_DOT ".mp3"
#define VIDEO_FILE_PATH "/mnt/tf/video/"
#define PHOTO_FILE_PATH "/mnt/tf/photo/"

#define MMCBLK_NAME "/dev/mmcblk0"
#define MMCBLKP1_NAME "/dev/mmcblk0p1"
#define MMCBLKP2_NAME "/dev/mmcblk0p2"

#ifdef SDCARD_PARTITION
#define MMCBLK_PATH "/dev/mmcblk0p1"
#define MMCBLK_PATH2 "/dev/mmcblk0p2"
#else
#define MMCBLK_PATH "/dev/mmcblk0"
#endif

#define SD_MIXED_CACHE_PATH SD_MIXED_PATH ".config"
#define SD_PHOTO_CACHE_PATH SD_PHOTO_PATH ".config"
#define SD_VIDEO_CACHE_PATH SD_VIDEO_PATH ".config"
#define SD_CALL_CACHE_PATH SD_CALL_PATH ".config"
#define SD_MSG_CACHE_PATH SD_MSG_PATH ".config"
#define SD_MOTION_CACHE_PATH SD_MOTION_PATH ".config"
#define SD_ALARM_CACHE_PATH SD_ALARM_PATH ".config"
#define FALSH_PHOTO_CACHE_PATH FLASH_PHOTO_PATH ".config"

static void sd_file_sync(media_type type);

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

static unsigned long long dir_total_size = 0;
int sum_size(const char *fpath, const struct stat *sb, int typeflag)
{
    if (typeflag == FTW_F)
    { // 处理文件
        dir_total_size += sb->st_size;
    }
    return 0; // 继续遍历
}

unsigned long long SD_card_file_size(media_type type)
{
    if (is_sdcard_insert() != true)
    {
        return -1;
    }

    char *dir_path = NULL;
    dir_total_size = 0;
    switch (type)
    {
    case FILE_TYPE_SD_CALL:
        dir_path = SD_CALL_PATH;
        break;
    case FILE_TYPE_SD_MSG:
        dir_path = SD_MSG_PATH;
        break;
    case FILE_TYPE_SD_MOTION:
        dir_path = SD_MOTION_PATH;
        break;

    default:
        break;
    }

    if (nftw(dir_path, (__nftw_func_t)sum_size, 20, FTW_PHYS) == -1)
    {
        perror("nftw");
        return 1;
    }
    Debug_Lib("type:%d Directory size is %llukb\n", type, dir_total_size / 1024);
    return dir_total_size;
}

void media_earliest_day_filename_get(media_info *Oldest_file, char *earliest_day)
{
    strncpy(earliest_day, Oldest_file->file_name, 8);
    earliest_day[8] = '\0';
    Debug_Lib("delete earliest_day:%s\n", earliest_day);
}

int media_earliest_day_filenum_get(char *earliest_day, char *path)
{
#if 1
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    dir = opendir(path);

    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, earliest_day, strlen(earliest_day)) == 0)
        {
            count++;
        }
        else
        {
            break;
        }
    }

    closedir(dir);
#else
    int count;
    char command[128];
    memset(command, 0, sizeof(command));
    sprintf(command, "find %s -type f -name \"%s*\" | wc -l", path, earliest_day);
    Debug_Lib("%s \n", command);

    char buffer[1024];
    FILE *pipe = popen(command, "r");
    if (pipe)
    {
        fgets(buffer, sizeof(buffer), pipe);
        pclose(pipe);
        count = atoi(buffer);
        Debug_Lib("%s ======>>%d\n", __func__, count);
    }
    else
    {
        Debug_Lib("Command failed.\n");
        return -1;
    }
#endif
    Debug_Lib(":%d,len:%dn", count, strlen(earliest_day));
    return count == 0 ? -1 : count;
}

bool media_tail_delete(media_type type, int index)
{
    media_info *info = NULL;
    int *total_file = NULL, *new_total_file = NULL;
    switch (type)
    {
    case FILE_TYPE_SD_CALL:
        info = &p_sd_call[index];
        total_file = &sd_call_total;
        new_total_file = &sd_call_new_total;
        break;
    case FILE_TYPE_SD_MSG:
        info = &p_sd_msg[index];
        total_file = &sd_msg_total;
        new_total_file = &sd_msg_new_total;
        break;
    case FILE_TYPE_SD_MOTION:
        info = &p_sd_motion[index];
        total_file = &sd_motion_total;
        new_total_file = &sd_motion_new_total;
        break;

    default:
        return false;
    }

    // Debug_Lib("index:%d ===================>>> \n",(*total_file - index)*sizeof(media_info));
    // Debug_Lib("total_file:%d ===================>>> \n",*total_file);
    // Debug_Lib("new_total_file:%d ===================>>> \n",*new_total_file);
    for (int i = 0; i < (*total_file - index); i++)
    {
        Debug_Lib("info[%d].is_new:%d name:%s  *new_total_file:%d===================>>> \n", i, info[i].is_new, info[i].file_name, *new_total_file);
        if ((info[i].is_new) && ((*new_total_file) > 0))
        {
            (*new_total_file)--;
        }
    }
    *total_file = index;
    memset(info, 0, (*total_file - index) * sizeof(media_info));

    sd_file_sync(type);
    return true;
}

static bool SD_card_space_flag = false;
#ifdef _DETECT_SPACE_
static unsigned long long call_dir_size = 0;
static unsigned long long msg_dir_size = 0;
static unsigned long long motion_dir_size = 100;
#endif
static void SD_card_space_clear(void)
{
    Debug_Lib("%s ===================>>> start\n", __func__);
    struct ak_timeval tv1, tv2;

    ak_get_ostime(&tv1);
    media_type type_temp = FILE_TYPE_SD_CALL;

#ifdef _DETECT_SPACE_
    int *total = &sd_call_total;
    char *path = SD_CALL_PATH;

    Debug_Lib("call_dir_size:%llu ===================>>> \n", call_dir_size);
    Debug_Lib("msg_dir_size:%llu ===================>>> \n", msg_dir_size);
    Debug_Lib("motion_dir_size:%llu ===================>>> \n", motion_dir_size);
    if ((call_dir_size == -1 && msg_dir_size == -1 && motion_dir_size == -1) || (call_dir_size | msg_dir_size | motion_dir_size) == 0)
    {
        Debug_Lib("%s ===================>>> fail\n", __func__);

        SD_card_space_flag = false;
        return;
    }

    if (call_dir_size < msg_dir_size)
    {
        type_temp = FILE_TYPE_SD_MSG;
        total = &sd_msg_total;
        path = SD_MSG_PATH;
        if (msg_dir_size < motion_dir_size)
        {
            type_temp = FILE_TYPE_SD_MOTION;
            total = &sd_motion_total;
            path = SD_MOTION_PATH;
        }
    }
    else if (call_dir_size < motion_dir_size)
    {
        type_temp = FILE_TYPE_SD_MOTION;
        total = &sd_motion_total;
        path = SD_MOTION_PATH;
    }

    int count;
    char earliest_file[MEDIA_PATH_MAX] = {0};
    char earliest_day[16] = {0};
    Debug_Lib("%s  clear type %d  \n", __func__, type_temp);
    media_info *Oldest_file = media_info_get(type_temp, 0);
    Debug_Lib("%s  total:%d Oldest_file %s  \n", __func__, *total, Oldest_file->file_name);

    media_earliest_day_filename_get(Oldest_file, earliest_day);

    strcpy(earliest_file, path);
    strcat(earliest_file, earliest_day);
    Debug_Lib("fine earliest_file:%s* !!!!!!!!!!!!!!!\n", earliest_file);
    if ((count = media_earliest_day_filenum_get(earliest_day, path)) != -1)
    {
        if (media_tail_delete(type_temp, *total - count))
        {
            char cmd[128];
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "rm -rf %s*", earliest_file);
            Debug_Lib("%s !!!!!!!!!!!!!!!\n", cmd);
            system(cmd);
        }
        else
        {
            Debug_Lib("media_tail_delete  fail count:%d!!!!!!!!!!!!!!!\n", count);
        }
    }
#else
    unsigned long long blocksize;
    unsigned long long freeDisk;

    if (sd_motion_total > sd_msg_total)
    {
        if (sd_motion_total < sd_call_total)
        {
            type_temp = FILE_TYPE_SD_CALL;
        }else
        type_temp = FILE_TYPE_SD_MOTION;
    }
    else
    {
        if (sd_msg_total < sd_call_total)
        {
            type_temp = FILE_TYPE_SD_CALL;
        }else
        type_temp = FILE_TYPE_SD_MSG;
    }

    while (1)
    {
        media_info *file = NULL;
        for (int i = 0; i < 500; i++)
        {
            if (is_sdcard_insert() == false)
            {
                goto exit;
            }

            file = media_info_get(type_temp, 0);
            if (file == NULL)
            {
                break;
            }

            record_file_type file_type = file->file_type;
            Debug_Lib("file delete type:%d ===================>>> \n", type_temp);
            if (media_file_delete(type_temp, 0) == -1)
            {
                break;
            }

            if (file_type == VIDEO_TYPE)
            {
                break;
            }
        }
        statfs(SD_BASE_PATH, &diskinfo);
        blocksize = diskinfo.f_bsize;
        freeDisk = diskinfo.f_bfree * blocksize;
        if (((freeDisk) >> 20) > 600)
        {
            break;
        }

        if (file == NULL)
        {
            if (sd_motion_total > sd_msg_total)
            {
                if (sd_motion_total < sd_call_total)
                {
                    type_temp = FILE_TYPE_SD_CALL;
                }else
                type_temp = FILE_TYPE_SD_MOTION;
            }
            else
            {
                if (sd_msg_total < sd_call_total)
                {
                    type_temp = FILE_TYPE_SD_CALL;
                }else
                type_temp = FILE_TYPE_SD_MSG;
            }

            if ((sd_motion_total | sd_call_total | sd_msg_total) == 0)
            {
                break;
            }
        }
    }

#endif
exit:
    ak_get_ostime(&tv2);
    statfs(SD_BASE_PATH, &diskinfo);
    blocksize = diskinfo.f_bsize;
    freeDisk = diskinfo.f_bfree * blocksize;
    Debug_Lib("%s ===================>>>end,takes %lu seconds,free space: %llu M\n", __func__, tv2.sec - tv1.sec, ((freeDisk) >> 20));
}

void detect_sd_free_space(void)
{
    // int count = 10;
    // while (count --)
    {
        statfs(SD_BASE_PATH, &diskinfo);
        statfs(SD_BASE_PATH2, &diskinfo_tuya);
        unsigned long long blocksize = diskinfo.f_bsize;
        unsigned long long totalsize = blocksize * diskinfo.f_blocks;
        unsigned long long freeDisk = diskinfo.f_bfree * blocksize;
        // unsigned long long availableDisk = diskinfo.f_bavail * blocksize;
        if (((totalsize) >> 20) > 1000 && ((freeDisk) >> 20) < 500)
        {
            // Debug_Lib("\033[1;31m blocksize:%llu \033[0m\n",blocksize);
            // Debug_Lib("\033[1;31m totalsize:%llu \033[0m\n",((totalsize)>>20));
            // Debug_Lib("\033[1;31m freeDisk:%llu \033[0m\n",freeDisk);
            // Debug_Lib("\033[1;31m availableDisk:%llu \033[0m\n",availableDisk);
            SD_card_space_flag = true;
        }
        Debug_Lib("\033[1;%dm The SD card memory space is insufficient %lluM. The SD card memory space is cleared. Procedure \033[0m\n", SD_card_space_flag ? 31 : 32, ((freeDisk) >> 20));
        // else{
        //     break;
        // }
    }
}

int sd_free_space_insufficient(void)
{
    unsigned long long total_blocks = diskinfo.f_bsize;
    unsigned long long freeDisk = diskinfo.f_bfree * total_blocks;
    return ((freeDisk) >> 20);
}

void get_SD_space(unsigned long *bavail, unsigned long *disk_all_space)
{
    unsigned long long total_blocks = diskinfo.f_bsize;

    *disk_all_space = (diskinfo.f_blocks * total_blocks) >> 20; // MB

    *bavail = (diskinfo.f_bavail * total_blocks) >> 20; // MB

    return;
}

static int _video_bad_path_check(const char *file, char *damaged_file)
{
    char bad_file_path[MEDIA_PATH_MAX];
    sprintf(bad_file_path, "%stemp", file);
    if (access(bad_file_path, F_OK) == 0)
    {
        if (damaged_file != NULL)
        {

            strcpy(damaged_file, bad_file_path);
        }
        return 1;
    }
    memset(bad_file_path, 0, sizeof(bad_file_path));
    sprintf(bad_file_path, "%s", file);
    if (access(bad_file_path, F_OK) != 0)
    {
        return 1;
    }
    return 0;
}
static int _photo_bad_path_check(const char *file, char *badfile)
{

    sprintf(badfile, "%s", file);
    if (access(badfile, F_OK) == 0)
    {
        return 0;
    }
    return 1;
}

static bool scan_find_file(media_type type, const char *dir_path, media_info *p_info, int *p_total, int *p_total_new)
{
    char cmd_buffer[64] = {0};
    if (type == FILE_TYPE_SD_CALL ||
        type == FILE_TYPE_SD_MSG ||
        type == FILE_TYPE_SD_MOTION ||
        type == FILE_TYPE_SD_ALARM)
    {
        sprintf(cmd_buffer, "ls %s*%s %s*%s", dir_path, PHOTO_DOT, dir_path, VIDEO_DOT);
    }
    else
    {
        sprintf(cmd_buffer, "find %s -type f", dir_path);
    }

    Debug_Lib("--- %s %d ---\n\r", cmd_buffer, strlen(cmd_buffer));
    FILE *pf = popen(cmd_buffer, "r");
    char buffer[128] = {0};
    while (fgets(buffer, 128, pf))
    {
        /**********************
        判断文件是否为规范
        **********************/
        buffer[strlen(buffer) - 1] = '\0';
        char *p_file = strrchr(buffer, '/') + 1;
        if (strlen(p_file) > 25 || strlen(p_file) < 4)
        {
            Debug_Lib("file name error: %s \n\r", p_file);
            goto fail_next;
        }
        /**************************
        判断是否为视频切换为坏文件
        ***************************/
        if (((type != FILE_TYPE_FLASH_PHOTO) && (type != FILE_TYPE_SD_PICTURE)) && (_video_bad_path_check(buffer, NULL) != 0))
        {

            Debug_Lib("unknown file type \n\r");
            goto fail_next;
        }

        // Debug_Lib("--- %s ---\n\r", p_file);
        /******************
        获取该文件所属类型
        *******************/
        char *ptr = strrchr(buffer, '.');
        if ((type == FILE_TYPE_SD_MIXED_PHOTO) && (strcmp(ptr, PHOTO_DOT) == 0))
        {
            p_info->type = FILE_TYPE_SD_MIXED_PHOTO;
        }
        else if ((type == FILE_TYPE_SD_MIXED_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0))
        {
            p_info->type = FILE_TYPE_SD_MIXED_VIDEO;
        }
        else if ((type == FILE_TYPE_SD_PHOTO) && (strcmp(ptr, PHOTO_DOT) == 0))
        {
            p_info->type = FILE_TYPE_SD_PHOTO;
        }
        else if ((type == FILE_TYPE_FLASH_PHOTO) && (strcmp(ptr, PHOTO_DOT) == 0))
        {
            p_info->type = FILE_TYPE_FLASH_PHOTO;
        }
        else if ((type == FILE_TYPE_SD_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0))
        {
            p_info->type = FILE_TYPE_SD_VIDEO;
        }
        else if ((type == FILE_TYPE_SD_MUSIC) && (strcmp(ptr, MUSIC_DOT) == 0))
        {
            p_info->type = FILE_TYPE_SD_MUSIC;
        }
        else if ((type == FILE_TYPE_SD_CALL) && ((strcmp(ptr, VIDEO_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
        {
            p_info->type = FILE_TYPE_SD_CALL;
        }
        else if ((type == FILE_TYPE_SD_MSG) && ((strcmp(ptr, VIDEO_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
        {
            p_info->type = FILE_TYPE_SD_MSG;
        }
        else if ((type == FILE_TYPE_SD_MOTION) && ((strcmp(ptr, VIDEO_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
        {
            p_info->type = FILE_TYPE_SD_MOTION;
        }
        else if ((type == FILE_TYPE_SD_ALARM) && ((strcmp(ptr, VIDEO_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
        {
            p_info->type = FILE_TYPE_SD_ALARM;
        }
        else if ((type == FILE_TYPE_SD_PICTURE) && ((strcmp(ptr, PICTURE_E_DOT) == 0) || (strcmp(ptr, PICTURE_DOT) == 0) || (strcmp(ptr, PHOTO_DOT) == 0)))
        {
            p_info->type = FILE_TYPE_SD_PICTURE;
        }
        else
        {
            goto fail_next;
        }

        /***********
        获取该文件名
        ************/
        strncpy(p_info->file_name, p_file, 23);

        p_info->ch = p_file[16] - 48;
        p_info->mode = p_file[17] - 48;
        p_info->file_type = p_file[18] - 48;
#if 0
		struct stat st;
        stat(buffer, &st);
        if (st.st_ctime == st.st_mtime) {
            p_info->is_new = 1;
            (*p_total_new)++;

        } else {
            p_info->is_new = 0;
        }
#endif
        p_info++;
        (*p_total)++;

        if ((type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && ((*p_total) > SD_MIXED_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_PHOTO) && ((*p_total) >= SD_PHOTO_MAX))
        {
            break;
        }
        if ((type == FILE_TYPE_SD_VIDEO) && ((*p_total) >= SD_VIDEO_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_MUSIC) && ((*p_total) > SD_MUSIC_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_PICTURE) && ((*p_total) > SD_PICTURE_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_CALL) && ((*p_total) > SD_RECORD_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_MSG) && ((*p_total) > SD_RECORD_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_MOTION) && ((*p_total) > SD_RECORD_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_SD_ALARM) && ((*p_total) > SD_RECORD_MAX))
        {
            break;
        }
        else if ((type == FILE_TYPE_FLASH_PHOTO) && ((*p_total) > FLASH_PHOTO_MAX))
        {
            break;
        }
    fail_next:
        memset(buffer, 0, sizeof(buffer));
    }
    pclose(pf);

    return true;
}

static void sd_media_file_load(media_type type, media_info *p_info, int *p_total, int *p_total_new, int max)
{
    FILE *fp = NULL;
    char *path = NULL;
    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
    {

        fp = fopen(SD_MIXED_CACHE_PATH, "rb");
        path = SD_MIXED_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_MIXED_CACHE_PATH);
            Debug_Lib(SD_MIXED_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_PHOTO)
    {
        fp = fopen(SD_PHOTO_CACHE_PATH, "rb");
        path = SD_PHOTO_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_PHOTO_CACHE_PATH);
            Debug_Lib(SD_PHOTO_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_VIDEO)
    {
        fp = fopen(SD_VIDEO_CACHE_PATH, "rb");
        path = SD_VIDEO_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_VIDEO_CACHE_PATH);
            Debug_Lib(SD_VIDEO_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_CALL)
    {
        fp = fopen(SD_CALL_CACHE_PATH, "rb");
        path = SD_CALL_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_CALL_CACHE_PATH);
            Debug_Lib(SD_CALL_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_MSG)
    {
        fp = fopen(SD_MSG_CACHE_PATH, "rb");
        path = SD_MSG_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_MSG_CACHE_PATH);
            Debug_Lib(SD_MSG_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_MOTION)
    {
        fp = fopen(SD_MOTION_CACHE_PATH, "rb");
        path = SD_MOTION_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_MOTION_CACHE_PATH);
            Debug_Lib(SD_MOTION_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_SD_ALARM)
    {
        fp = fopen(SD_ALARM_CACHE_PATH, "rb");
        path = SD_ALARM_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " SD_ALARM_CACHE_PATH);
            Debug_Lib(SD_ALARM_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else if (type == FILE_TYPE_FLASH_PHOTO)
    {
        fp = fopen(FALSH_PHOTO_CACHE_PATH, "rb");
        path = FALSH_PHOTO_CACHE_PATH;
        if (fp == NULL)
        {
            system("touch " FALSH_PHOTO_CACHE_PATH);
            Debug_Lib(FALSH_PHOTO_CACHE_PATH " not exit \n\r");
            return;
        }
    }
    else
    {
        return;
    }

    int read_len = 0;
    media_info info;
    int read_size = sizeof(media_info);

    // struct ak_timeval cur_t,prev_t;
    // char *dir_path = NULL;
    // switch (type)
    // {
    // case FILE_TYPE_SD_CALL:
    //     dir_path = SD_CALL_PATH;
    //     break;
    // case FILE_TYPE_SD_MSG:
    //     dir_path = SD_MSG_PATH;
    //     break;
    // case FILE_TYPE_SD_MOTION:
    //     dir_path = SD_MOTION_PATH;
    //     break;

    // default:

    //     break;
    // }

    while ((read_len = fread(&info, 1, read_size, fp)) == read_size)
    {
        *p_info = info;
        if (info.file_name[0] == 0)
        {
            fclose(fp);
            if (path != NULL)
            {
                char cmd[128] = {0};
                sprintf(cmd, "rm -f %s", path);
                Debug_Lib("%s\n", cmd);
                system(cmd);
                system("sync");
            }
            break;
        }
        /* 执行一次需要100多us，时间太久关闭此功能 */
        // ak_get_ostime(&prev_t);
        // if(dir_path != NULL)
        // {
        //     Debug_Lib("access function  type:%d info.file_name:%s\n",type,info.file_name);
        //     char path[128] = {0};
        //     sprintf(path,"%s%s",dir_path,info.file_name);
        //     if(access(path,F_OK) != 0)
        //     {
        //         continue;
        //     }
        //     ak_get_ostime(&cur_t);
        //     Debug_Lib("access function   info.file_name:%s exist  cur_t:%ld   prev_t:%ld\n",path,cur_t.usec,prev_t.usec);
        // }

        (*p_total)++;
        if (info.is_new)
        {
            (*p_total_new)++;
        }
        if ((*p_total) >= max)
        {
            break;
        }
        // Debug_Lib("lock :%s\n",info.is_lock?"ture":"false");
        p_info++;
    }
    // sd_file_sync(type);
    fclose(fp);
    Debug_Lib("media total:%d total_new:%d\n\r", (*p_total), (*p_total_new));
}

static void sd_file_sync(media_type type)
{
    FILE *fp;
    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
    {
        fp = fopen(SD_MIXED_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_MIXED_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_mixed, sd_mixed_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_SD_PHOTO)
    {
        fp = fopen(SD_PHOTO_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_PHOTO_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_photo, sd_photo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_SD_CALL)
    {
        fp = fopen(SD_CALL_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_CALL_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_call, sd_call_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_SD_MSG)
    {
        fp = fopen(SD_MSG_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_MSG_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_msg, sd_msg_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_SD_MOTION)
    {
        fp = fopen(SD_MOTION_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_MOTION_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_motion, sd_motion_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_SD_ALARM)
    {
        fp = fopen(SD_ALARM_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_ALARM_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_alarm, sd_alarm_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else if (type == FILE_TYPE_FLASH_PHOTO)
    {
        fp = fopen(FALSH_PHOTO_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(FALSH_PHOTO_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_flash_photo, flash_photo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
    else
    {
        fp = fopen(SD_VIDEO_CACHE_PATH, "wb");
        if (fp == NULL)
        {

            Debug_Lib(SD_VIDEO_CACHE_PATH "not exit \n\r");
            return;
        }
        fwrite(p_sd_video, sd_video_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }
}

static int scan_media_file(media_type type)
{

    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && (access(SD_MIXED_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_MIXED_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_PHOTO) && (access(SD_PHOTO_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_PHOTO_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_VIDEO) && (access(SD_VIDEO_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_VIDEO_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_MUSIC) && (access(SD_MUSIC_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_MUSIC_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_PICTURE) && (access(SD_PICTURE_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_PICTURE_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_CALL) && (access(SD_CALL_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_CALL_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_MSG) && (access(SD_MSG_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_MSG_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_MOTION) && (access(SD_MOTION_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_MOTION_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_SD_ALARM) && (access(SD_ALARM_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", SD_ALARM_PATH);
        return -1;
    }
    else if ((type == FILE_TYPE_FLASH_PHOTO) && (access(FLASH_PHOTO_PATH, F_OK) != 0))
    {

        Debug_Lib("%s non-existent \n\r", FLASH_PHOTO_PATH);
        system("mkdir " FLASH_PHOTO_PATH);
        return -1;
    }
    else if (type >= FILE_TYPE_NONE)
    {

        Debug_Lib("unknown file type \n\r");
        return -1;
    }

    char *dir_path = NULL;
    media_info *p_array = NULL;
    int *p_total = NULL;
    int *p_new_total = NULL;

    Debug_Lib("scan_media_file type: %d \n\r", type);
    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
    {

        dir_path = SD_MIXED_PATH;
        sd_mixed_total = sd_mixed_new_total = 0;
        p_array = p_sd_mixed;
        p_total = &sd_mixed_total;
        p_new_total = &sd_mixed_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_MIXED_MAX);
    }
    else if (type == FILE_TYPE_SD_PHOTO)
    {

        dir_path = SD_PHOTO_PATH;
        sd_photo_total = sd_photo_new_total = 0;
        p_array = p_sd_photo;
        p_total = &sd_photo_total;
        p_new_total = &sd_photo_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_PHOTO_MAX);
    }
    else if (type == FILE_TYPE_SD_VIDEO)
    {

        dir_path = SD_VIDEO_PATH;
        sd_video_total = sd_video_new_total = 0;
        p_array = p_sd_video;
        p_total = &sd_video_total;
        p_new_total = &sd_video_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_VIDEO_MAX);
    }
    else if (type == FILE_TYPE_SD_MUSIC)
    {

        dir_path = SD_MUSIC_PATH;
        sd_music_total = sd_music_new_total = 0;
        p_array = p_sd_music;
        p_total = &sd_music_total;
        p_new_total = &sd_music_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
    }
    else if (type == FILE_TYPE_SD_PICTURE)
    {

        dir_path = SD_PICTURE_PATH;
        sd_picture_total = sd_picture_new_total = 0;
        p_array = p_sd_picture;
        p_total = &sd_picture_total;
        p_new_total = &sd_picture_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
    }
    else if (type == FILE_TYPE_SD_CALL)
    {

        dir_path = SD_CALL_PATH;
        sd_call_total = sd_call_new_total = 0;
        p_array = p_sd_call;
        p_total = &sd_call_total;
        p_new_total = &sd_call_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_RECORD_MAX);
    }
    else if (type == FILE_TYPE_SD_MSG)
    {

        dir_path = SD_MSG_PATH;
        sd_msg_total = sd_msg_new_total = 0;
        p_array = p_sd_msg;
        p_total = &sd_msg_total;
        p_new_total = &sd_msg_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_RECORD_MAX);
    }
    else if (type == FILE_TYPE_SD_MOTION)
    {

        dir_path = SD_MOTION_PATH;
        sd_motion_total = sd_motion_new_total = 0;
        p_array = p_sd_motion;
        p_total = &sd_motion_total;
        p_new_total = &sd_motion_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_RECORD_MAX);
    }
    else if (type == FILE_TYPE_SD_ALARM)
    {

        dir_path = SD_ALARM_PATH;
        sd_alarm_total = sd_alarm_new_total = 0;
        p_array = p_sd_alarm;
        p_total = &sd_alarm_total;
        p_new_total = &sd_alarm_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, SD_RECORD_MAX);
    }
    else if (type == FILE_TYPE_FLASH_PHOTO)
    {
        dir_path = FLASH_PHOTO_PATH;
        flash_photo_total = flash_photo_new_total = 0;
        p_array = p_flash_photo;
        p_total = &flash_photo_total;
        p_new_total = &flash_photo_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total, FLASH_PHOTO_MAX);
        system("sync");
        //  scan_find_file(type, dir_path, p_array, p_total, p_new_total);
    }

    return 0;
}

static void sd_file_create(void)
{
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if (access(SD_MIXED_PATH, F_OK) != 0)
        {

            system("mkdir " SD_MIXED_PATH);
        }
    }
    else
    {
        if (access(SD_PHOTO_PATH, F_OK) != 0)
        {

            system("mkdir " SD_PHOTO_PATH);
        }
        if (access(SD_VIDEO_PATH, F_OK) != 0)
        {

            system("mkdir " SD_VIDEO_PATH);
        }
    }

    if ((access(SD_MUSIC_PATH, F_OK) != 0) && BG_MIUSIC_FILE_ENABLE)
    {
        system("mkdir " SD_MUSIC_PATH);
    }
    if ((access(SD_PICTURE_PATH, F_OK) != 0) && BG_PICTURE_FILE_ENABLE)
    {
        system("mkdir " SD_PICTURE_PATH);
    }
    if ((access(SD_CALL_PATH, F_OK) != 0) && REC_CALL_FILE_ENABLE)
    {
        system("mkdir " SD_CALL_PATH);
    }
    if ((access(SD_MSG_PATH, F_OK) != 0) && REC_MSG_FILE_ENABLE)
    {
        system("mkdir " SD_MSG_PATH);
    }
    if ((access(SD_MOTION_PATH, F_OK) != 0) && REC_MOTION_FILE_ENABLE)
    {
        system("mkdir " SD_MOTION_PATH);
    }
    if ((access(SD_ALARM_PATH, F_OK) != 0) && REC_ALARM_FILE_ENABLE)
    {
        system("mkdir " SD_ALARM_PATH);
    }
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        scan_media_file(FILE_TYPE_SD_MIXED);
    }
    else
    {
        scan_media_file(FILE_TYPE_SD_PHOTO);
        scan_media_file(FILE_TYPE_SD_VIDEO);
    }
    if (BG_MIUSIC_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_MUSIC);
    }
    if (BG_PICTURE_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_PICTURE);
    }
    if (REC_CALL_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_CALL);
    }
    if (REC_MSG_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_MSG);
    }
    if (REC_MOTION_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_MOTION);
    }
    if (REC_ALARM_FILE_ENABLE)
    {
        scan_media_file(FILE_TYPE_SD_ALARM);
    }
    system("sync");
}
/* 1:格式化室内机分区，2:格式化涂鸦分区 ,3.新建并格式化所有分区*/
int format_sd_flag = 0;
extern bool _sdcard_format(int);
static void sd_format_process(void)
{
    system("cd /");
#ifdef SDCARD_PARTITION
    if (format_sd_flag == 2)
    {
        system("rm -rf /mnt/tf2/*");

        system("umount " SD_BASE_PATH2);
        _sdcard_format(2);

        if (access(SD_BASE_PATH2, F_OK) != 0)
        {

            system("mkdir " SD_BASE_PATH2);
        }
        else
        {

            system("mount -t vfat " MMCBLK_PATH2 " " SD_BASE_PATH2 " -o errors=continue");
        }
        return;
    }
#endif

    system("rm -rf /mnt/tf/*");
    system("umount " SD_BASE_PATH);
    _sdcard_format(1);

    if (access(SD_BASE_PATH, F_OK) != 0)
    {

        system("mkdir " SD_BASE_PATH);
    }
    else
    {

        system("mount -t vfat " MMCBLK_PATH " " SD_BASE_PATH " -o errors=continue");
    }
    sd_file_create();
}

bool copy_to_sd_flag = false;
static void copy_to_sd_process(void)
{
    if (access(SD_BACKUP_PATH, F_OK) != 0)
    {

        char buf[128] = {0};
        sprintf(buf, "mkdir %s", SD_BACKUP_PATH);
        system(buf);
        ak_sleep_ms(20);
        system("sync");
    }
    system("mv " FLASH_PHOTO_PATH "*.JPG " SD_BACKUP_PATH);
    ak_sleep_ms(20);
    system("sync");
    ak_sleep_ms(20);
}

int delete_file_flag = 0;
static void delete_file_process(void)
{
    if (delete_file_flag & DELETE_ALL_SD_PHOTO)
    { /// photo

        if (access(SD_PHOTO_PATH, F_OK) == 0)
        {

            system("rm " SD_PHOTO_CACHE_PATH);
            system("rm -rf " SD_PHOTO_PATH "*JPG");

            scan_media_file(FILE_TYPE_SD_PHOTO);
            Debug_Lib("\033[31m delete sd photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_SD_VIDEO)
    { /// video

        if (access(SD_VIDEO_PATH, F_OK) == 0)
        {

            system("rm " SD_VIDEO_CACHE_PATH);
            system("rm -rf " SD_VIDEO_PATH "*AVI*");

            scan_media_file(FILE_TYPE_SD_VIDEO);

            Debug_Lib("\033[31m delete sd video \n\r");
        }
    }
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if (delete_file_flag & DELETE_ALL_MIXED)
        { /// mixed

            if (access(SD_MIXED_PATH, F_OK) == 0)
            {

                system("rm " SD_MIXED_CACHE_PATH);
                system("rm -rf " SD_MIXED_PATH "*.*");
                scan_media_file(FILE_TYPE_SD_MIXED);
                Debug_Lib("\033[31m delete sd mixed file \n\r");
            }
        }

        if (delete_file_flag & DELETE_ALL_MIXED_PHOTO)
        {
            for (int i = 0; i < sd_mixed_total; i++)
            {
                media_info *media = media_info_get(FILE_TYPE_SD_MIXED, i);
                if (media->type == FILE_TYPE_SD_MIXED_PHOTO)
                {
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    i--;
                }
            }
        }

        if (delete_file_flag & DELETE_ALL_MIXED_VIDEO)
        {
            for (int i = 0; i < sd_mixed_total; i++)
            {
                media_info *media = media_info_get(FILE_TYPE_SD_MIXED, i);
                if (media->type == FILE_TYPE_SD_MIXED_VIDEO)
                {
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    i--;
                }
            }
        }
        scan_media_file(FILE_TYPE_SD_MIXED);
    }

    if (delete_file_flag & DELETE_ALL_FLASH_PHOTO)
    { /// photo

        if (access(FLASH_PHOTO_PATH, F_OK) == 0)
        {
            system("rm -rf " FLASH_PHOTO_PATH "*JPG");
            system("rm -rf " FLASH_PHOTO_PATH "*jpg");

            system("rm " FALSH_PHOTO_CACHE_PATH);
            scan_media_file(FILE_TYPE_FLASH_PHOTO);

            Debug_Lib("\033[31m delete flash photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_MUSIC)
    { /// music
        if (access(SD_MUSIC_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_MUSIC_PATH "*MP3");
            system("rm -rf " SD_MUSIC_PATH "*mp3");
            scan_media_file(FILE_TYPE_SD_MUSIC);

            Debug_Lib("\033[31m delete sd music \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_PICTURE)
    {
        if (access(SD_PICTURE_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_PICTURE_PATH "*JPG");
            system("rm -rf " SD_PICTURE_PATH "*jpg");
            scan_media_file(FILE_TYPE_SD_PICTURE);

            Debug_Lib("\033[31m delete sd music \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_CALL)
    { ///
        if (access(SD_CALL_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_CALL_PATH "*AVI");
            system("rm -rf " SD_CALL_PATH "*JPG");
            system("rm -rf " SD_CALL_CACHE_PATH);
            scan_media_file(FILE_TYPE_SD_CALL);

            Debug_Lib("\033[31m delete sd call :%d\n\r", sd_call_total);
        }
    }
    if (delete_file_flag & DELETE_ALL_MSG)
    {
        if (access(SD_MSG_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_MSG_PATH "*JPG");
            system("rm -rf " SD_MSG_PATH "*AVI");
            system("rm -rf " SD_MSG_CACHE_PATH);
            scan_media_file(FILE_TYPE_SD_MSG);

            Debug_Lib("\033[31m delete sd msg \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_MOTION)
    { ///
        if (access(SD_MOTION_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_MOTION_PATH "*AVI");
            system("rm -rf " SD_MOTION_PATH "*JPG");
            system("rm -rf " SD_MOTION_CACHE_PATH);
            scan_media_file(FILE_TYPE_SD_MOTION);

            Debug_Lib("\033[31m delete sd motion \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_ALARM)
    {
        if (access(SD_ALARM_PATH, F_OK) == 0)
        {
            system("rm -rf " SD_ALARM_PATH "*JPG");
            system("rm -rf " SD_ALARM_PATH "*AVI");
            system("rm -rf " SD_ALARM_CACHE_PATH);
            scan_media_file(FILE_TYPE_SD_ALARM);

            Debug_Lib("\033[31m delete sd alarm \n\r");
        }
    }

    if (delete_file_flag)
    {
        Debug_Lib("\033[31m delete file succeed!!!!!!!!!!!!!!!!!\n\r");
        fflush(stdout);
        ak_sleep_ms(20);
        system("sync");
        delete_file_flag = 0;
    }
}

#include <sys/mount.h>
bool umount_mmcblk(void)
{
    char umount_str[128] = {0};
    sprintf(umount_str, "umount %s", SD_BASE_PATH);
    system(umount_str);
    Debug_Lib("\n\n\n:%s\n", umount_str);

#ifdef SDCARD_PARTITION
    memset(umount_str, 0, sizeof(umount_str));
    sprintf(umount_str, "umount %s", SD_BASE_PATH2);
    system(umount_str);
    Debug_Lib("\n\n\n:%s\n", umount_str);
#endif
    return true;
}

bool _mount_mmcblk(int index)
{
    int ret = false;
    bool result = true;
    char *options = "errors=continue"; // 挂载选项
    char *mmcblk_name = NULL;
    char *mmcblk_path = NULL;
    switch (index)
    {
    case 0:
        mmcblk_name = MMCBLK_NAME;
        mmcblk_path = SD_BASE_PATH0;
        break;
    case 1:
        mmcblk_name = MMCBLKP1_NAME;
        mmcblk_path = SD_BASE_PATH;
        break;
    case 2:
        mmcblk_name = MMCBLKP2_NAME;
        mmcblk_path = SD_BASE_PATH2;
        break;

    default:
        return false;
    }
    ret = mount(mmcblk_name, mmcblk_path, "vfat", 0, options); // 挂载VFAT分区
    if (ret == 0)
    {
        Debug_Lib("Mount %s successful\n", mmcblk_name);
    }
    else
    {
        Debug_Lib("Mount %s failed\n", mmcblk_name);
        perror("Mount  failed\n");
        result = false;
    }
    return result;
}
bool mount_mmcblk(void)
{
    int ret;
    bool result = true;
    char *options = "errors=continue"; // 挂载选项

    // #ifdef SDCARD_PARTITION
    //  _sd_card_repartition_main();
    // #endif
    if (access(MMCBLKP1_NAME, F_OK) == 0)
    {
        ret = mount(MMCBLKP1_NAME, SD_BASE_PATH, "vfat", 0, options); // 挂载VFAT分区
        if (ret == 0)
        {
            Debug_Lib("Mount %s successful\n", MMCBLK_PATH);
        }
        else
        {
            perror("Mount  failed\n");
            result = false;
        }
    }
    else
    {
        ret = mount(MMCBLK_NAME, SD_BASE_PATH, "vfat", 0, options); // 挂载VFAT分区
        if (ret == 0)
        {
            Debug_Lib("Mount %s successful\n", MMCBLK_NAME);
        }
        else
        {
            perror("Mount  MMCBLK_NAME failed\n");
            result = false;
        }
    }
#ifdef SDCARD_PARTITION

    ret = mount(MMCBLK_PATH2, SD_BASE_PATH2, "vfat", 0, options); // 挂载VFAT分区
    if (ret == 0)
    {
        Debug_Lib("Mount %s successful\n", MMCBLK_PATH2);
    }
    else
    {
        perror("Mount MMCBLK_PATH2 failed\n");
        result = false;
    }
#endif
    return result;
}

bool is_mounted(const char *source, const char *target)
{
    struct stat st;
    if (stat(target, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void *file_list_task(void *arg)
{
    char cur_insert = 0;
    bool sdcard_format_error = false;
    struct ak_timeval curr_t, prev_t /* ,cur_t */;
    ak_get_ostime(&prev_t);
    while (1)
    {

        ak_get_ostime(&curr_t);
        cur_insert = (access(MMCBLK_NAME, F_OK) == 0) ? 1 : 0;
        if (sdcard_format_error && cur_insert && ak_diff_ms_time(&curr_t, &prev_t) > 2000 && format_sd_flag == 0)
        {
            prev_t = curr_t;
            extern bool sdcard_status_change_push(char, char);
            sdcard_status_change_push(2, 3);
        }

        if (cur_insert != sd_card_inserted)
        {

            ak_thread_mutex_lock(&file_list_mutex);
            sd_card_inserted = cur_insert;
            if (sd_card_inserted)
            {

                if (access(SD_BASE_PATH0, F_OK) != 0)
                {

                    system("mkdir " SD_BASE_PATH0);
                }

                if (access(SD_BASE_PATH, F_OK) != 0)
                {

                    system("mkdir " SD_BASE_PATH);
                }

#ifdef SDCARD_PARTITION
                if (access(SD_BASE_PATH2, F_OK) != 0)
                {

                    system("mkdir " SD_BASE_PATH2);
                }
#endif

                extern bool _sdcard_info_check_valid(void);
                if (_sdcard_info_check_valid() == false)
                {
                    sdcard_format_error = true;
                    extern bool sdcard_status_change_push(char, char);
                    sdcard_status_change_push(2, 3);
                    ak_thread_mutex_unlock(&file_list_mutex);
                    ak_sleep_ms(50);
                    continue;
                }
                mount_mmcblk();

                statfs(SD_BASE_PATH, &diskinfo);
                statfs(SD_BASE_PATH2, &diskinfo_tuya);
                unsigned long long blocksize = diskinfo.f_bsize;
                unsigned long long totalsize = blocksize * diskinfo.f_blocks;
                // unsigned long long availableDisk = diskinfo.f_bavail * blocksize;
                bool ret = (((totalsize) >> 20) < 100) ? false : true;
                Debug_Lib("%s ==============================>>>>%lld\n", __func__, ((totalsize) >> 20));
                if (ret == false)
                {
                    sdcard_status_change_push(2, 3);
                    ak_thread_mutex_unlock(&file_list_mutex);
                    continue;
                }

                sdcard_status_change_push(1, 0);
                if (MIX_PHOTOS_AND_VIDEOS_FILE)
                {
                    if (access(SD_MIXED_PATH, F_OK) != 0)
                    {

                        system("mkdir " SD_MIXED_PATH);
                    }
                    scan_media_file(FILE_TYPE_SD_MIXED);
                }
                else
                {
                    if (access(SD_PHOTO_PATH, F_OK) != 0)
                    {

                        system("mkdir " SD_PHOTO_PATH);
                    }
                    if (access(SD_VIDEO_PATH, F_OK) != 0)
                    {

                        system("mkdir " SD_VIDEO_PATH);
                    }
                    scan_media_file(FILE_TYPE_SD_PHOTO);
                    scan_media_file(FILE_TYPE_SD_VIDEO);
                }

                if ((access(SD_MUSIC_PATH, F_OK) != 0) && BG_MIUSIC_FILE_ENABLE)
                {
                    system("mkdir " SD_MUSIC_PATH);
                }
                if ((access(SD_PICTURE_PATH, F_OK) != 0) && BG_PICTURE_FILE_ENABLE)
                {
                    system("mkdir " SD_PICTURE_PATH);
                }
                if ((access(SD_CALL_PATH, F_OK) != 0) && REC_CALL_FILE_ENABLE)
                {
                    system("mkdir " SD_CALL_PATH);
                }
                if ((access(SD_MSG_PATH, F_OK) != 0) && REC_MSG_FILE_ENABLE)
                {
                    system("mkdir " SD_MSG_PATH);
                }
                if ((access(SD_MOTION_PATH, F_OK) != 0) && REC_MOTION_FILE_ENABLE)
                {
                    system("mkdir " SD_MOTION_PATH);
                }
                if ((access(SD_ALARM_PATH, F_OK) != 0) && REC_ALARM_FILE_ENABLE)
                {
                    system("mkdir " SD_ALARM_PATH);
                }
                scan_media_file(FILE_TYPE_SD_MUSIC);
                scan_media_file(FILE_TYPE_SD_PICTURE);
                scan_media_file(FILE_TYPE_SD_CALL);
                scan_media_file(FILE_TYPE_SD_MSG);
                scan_media_file(FILE_TYPE_SD_MOTION);
                scan_media_file(FILE_TYPE_SD_ALARM);

                /* 等待扫描结束再发送事件 */
                extern bool sdcard_status_change_push(char, char);
                sdcard_status_change_push(sd_card_inserted, 1);
            }
            else
            {
                // umount_mmcblk();
                bool _umount_mnt(void);
                _umount_mnt();
                sdcard_format_error = false;
                extern bool sdcard_status_change_push(char, char);
                sdcard_status_change_push(sd_card_inserted, 0x00);
            }

            ak_thread_mutex_unlock(&file_list_mutex);
        }
        if (format_sd_flag)
        {
            extern bool sdcard_status_change_push(char, char);
            if (format_sd_flag == 3)
            {
                sdcard_status_change_push(2, 1);
            }

            extern bool mount_mmcblk(void);
            mount_mmcblk();
            sdcard_upgrade_package_copy(MNT_UPGRADE_PACKAGR_PATH, TMP_UPGRADE_PACKAGR_PATH, false);
            Debug_Lib("\n\n\n");
            int ret = _sd_card_repartition_main();
            if (access(MMCBLKP1_NAME, F_OK) == 0)
            {
                if (_mount_mmcblk(0))
                    sdcard_upgrade_package_copy(TMP_UPGRADE_PACKAGR_PATH, MNT0_UPGRADE_PACKAGR_PATH, false);

                Debug_Lib("\n\n\n");
                bool _umount_mnt(void);
                _umount_mnt();
                ak_sleep_ms(100);
            }
            Debug_Lib("\n\n\n");
            mount_mmcblk();
            sdcard_upgrade_package_copy(TMP_UPGRADE_PACKAGR_PATH, MNT_UPGRADE_PACKAGR_PATH, true);

            if (format_sd_flag == 3)
            {
                sdcard_status_change_push(2, 2);
                sd_file_create();
                statfs(SD_BASE_PATH, &diskinfo);
                statfs(SD_BASE_PATH2, &diskinfo_tuya);
                sdcard_format_error = false;
            }

            if (ret == false)
            {
                sd_format_process();
            }

            format_sd_flag = 0;
        }
        if (copy_to_sd_flag)
        {
            Debug_Lib("----------------- 123\n\r");
            copy_to_sd_process();
            scan_media_file(FILE_TYPE_FLASH_PHOTO);
            copy_to_sd_flag = false;
        }
        if (delete_file_flag)
        {
            delete_file_process();
        }
        if (SD_card_space_flag)
        {
            SD_card_space_clear();
            SD_card_space_flag = false;
        }
        ak_sleep_ms(50);
    }
    ak_thread_exit();
    return NULL;
}

#ifdef _DETECT_SPACE_
static void *sdcard_space_det_task(void *arg)
{
    struct ak_timeval tv1, tv2;
    ak_get_ostime(&tv1);
    while (1)
    {
        ak_get_ostime(&tv2);
        if ((tv2.sec - tv1.sec) > 120)
        {
            tv1 = tv2;
            call_dir_size = SD_card_file_size(FILE_TYPE_SD_CALL);
            msg_dir_size = SD_card_file_size(FILE_TYPE_SD_MSG);
            motion_dir_size = SD_card_file_size(FILE_TYPE_SD_MOTION);
        }

        ak_sleep_ms(10);
    }
    ak_thread_exit();
    return NULL;
}
#endif

void start_format_sd_card(int type)
{
    format_sd_flag = type;
}
bool format_sd_card_status(void)
{
    return format_sd_flag ? true : false;
}
void start_copy_flash_photo_to_sd(void)
{
    copy_to_sd_flag = true;
}
bool copy_flash_photo_to_sd_status(void)
{
    return copy_to_sd_flag;
}
void start_delete_media(enum delete_flag item)
{
    delete_file_flag = item;
    Debug_Lib("delete_file_flag ===============.%d\n", delete_file_flag);
}
int delete_media_status(void)
{
    return delete_file_flag;
}

void forced_scan_media_file(media_type type)
{
    if (is_sdcard_insert() == false)
        return;

    scan_media_file(type);
}

void media_file_list_init(void)
{
    // #ifdef SDCARD_PARTITION
    // _sd_card_repartition_main();
    // #endif
    // umount_mmcblk();
    bool _umount_mnt(void);
    _umount_mnt();
    // ak_sleep_ms(100);
    // mount_mmcblk();
    ak_thread_mutex_init(&file_list_mutex, NULL);
    p_flash_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + FLASH_PHOTO_MAX));
    scan_media_file(FILE_TYPE_FLASH_PHOTO);
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        p_sd_mixed = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_MIXED_MAX));
        scan_media_file(FILE_TYPE_SD_MIXED);
    }
    else
    {
        p_sd_video = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_VIDEO_MAX));
        p_sd_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_PHOTO_MAX));
        scan_media_file(FILE_TYPE_SD_PHOTO);
        scan_media_file(FILE_TYPE_SD_VIDEO);
    }
    if (BG_MIUSIC_FILE_ENABLE)
    {
        p_sd_music = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_MUSIC_MAX));
        scan_media_file(FILE_TYPE_SD_MUSIC);
    }
    if (BG_PICTURE_FILE_ENABLE)
    {
        p_sd_picture = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_PICTURE_MAX));
        scan_media_file(FILE_TYPE_SD_PICTURE);
    }
    if (REC_CALL_FILE_ENABLE)
    {
        p_sd_call = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_RECORD_MAX));
        scan_media_file(FILE_TYPE_SD_CALL);
    }
    if (REC_MSG_FILE_ENABLE)
    {
        p_sd_msg = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_RECORD_MAX));
        scan_media_file(FILE_TYPE_SD_MSG);
    }
    if (REC_MOTION_FILE_ENABLE)
    {
        p_sd_motion = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_RECORD_MAX));
        scan_media_file(FILE_TYPE_SD_MOTION);
    }
    if (REC_ALARM_FILE_ENABLE)
    {
        p_sd_alarm = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * (1 + SD_RECORD_MAX));
        scan_media_file(FILE_TYPE_SD_ALARM);
    }
    ak_pthread_t task_id;
    ak_thread_create(&task_id, file_list_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
#ifdef _DETECT_SPACE_
    ak_pthread_t sdcard_space_task;
    ak_thread_create(&sdcard_space_task, sdcard_space_det_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
#endif
}

bool is_sdcard_insert(void)
{

    //  ak_thread_mutex_lock(&file_list_mutex);
    char insert = sd_card_inserted;
    //    ak_thread_mutex_unlock(&file_list_mutex);
    return insert ? true : false;
}

bool create_one_media_file(media_type type, char ch, char mode, record_file_type file_type, char *path)
{
    if ((type != FILE_TYPE_FLASH_PHOTO) && ((is_sdcard_insert() == 0) || format_sd_card_status()))
    {
        media_file_delete(type, 0);
        return false;
    }

    if (SD_card_space_flag)
    {
        return false;
    }
    // if(is_sdcard_insert() && sd_free_space_insufficient() == false)
    // {
    //     extern bool sdcard_status_change_push(char,char);
    //     sdcard_status_change_push(sd_card_inserted,0x01);
    //     return false;
    // }
    media_info *p_array = NULL;
    char *file_path = NULL;
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && MIX_PHOTOS_AND_VIDEOS_FILE)
    {

        if (sd_mixed_total >= SD_MIXED_MAX)
        {
            int i = 0;
            int total = sd_mixed_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_mixed[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total) // 全部上锁，无法删除最旧一张
            {
                return false;
            }
        }
        file_path = SD_MIXED_PATH;
        p_array = &p_sd_mixed[sd_mixed_total];
        sd_mixed_total++;
        sd_mixed_new_total++;
    }
    else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {
        Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
        if (sd_video_total >= SD_VIDEO_MAX)
        {
            int total = sd_video_total;
            int i = 0;
            for (i = 0; i < total; i++)
            {
                if (p_sd_video[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
                Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
            }
        }
        file_path = SD_VIDEO_PATH;
        p_array = &p_sd_video[sd_video_total];
        sd_video_total++;
        sd_video_new_total++;
    }
    else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {

        Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
        if (sd_photo_total >= SD_VIDEO_MAX)
        {
            int i = 0;
            int total = sd_photo_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_photo[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
                    break;
                }
            }
            if (i == total)
            {
                Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
                return false;
            }
        }
        file_path = SD_PHOTO_PATH;
        p_array = &p_sd_photo[sd_photo_total];
        sd_photo_total++;
        sd_photo_new_total++;
    }
    else if ((type == FILE_TYPE_SD_MUSIC) && BG_MIUSIC_FILE_ENABLE)
    {
        Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
        file_path = SD_MUSIC_PATH;
        p_array = &p_sd_music[sd_music_total];
        sd_music_total++;
        sd_music_new_total++;
        if (sd_music_total > SD_MUSIC_MAX)
        {
            Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
            media_file_delete(type, 0);
        }
    }
    else if ((type == FILE_TYPE_SD_PICTURE) && BG_PICTURE_FILE_ENABLE)
    {
        Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
        file_path = SD_PICTURE_PATH;
        p_array = &p_sd_picture[sd_picture_total];
        sd_picture_total++;
        sd_picture_new_total++;
        if (sd_picture_total > SD_PICTURE_MAX)
        {
            Debug_Lib("%s============================>>%d \n\r", __func__, __LINE__);
            media_file_delete(type, 0);
        }
    }
    else if (type == FILE_TYPE_FLASH_PHOTO)
    {
        if (flash_photo_total >= FLASH_PHOTO_MAX)
        {
            int i = 0;
            int total = flash_photo_total;
            for (i = 0; i < total; i++)
            {
                if (p_flash_photo[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
            }
        }
        file_path = FLASH_PHOTO_PATH;
        p_array = &p_flash_photo[flash_photo_total];
        flash_photo_total++;
        flash_photo_new_total++;
    }
    else if (type == FILE_TYPE_SD_CALL)
    {
        if (sd_call_total >= SD_RECORD_MAX)
        {
            int i = 0;
            int total = sd_call_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_call[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
            }
        }
        file_path = SD_CALL_PATH;
        p_array = &p_sd_call[sd_call_total];
        Debug_Lib("%s============================>>%d,type:%d,p_array:%p \n\r", __func__, __LINE__, type, p_array);
        sd_call_total++;
        sd_call_new_total++;
    }
    else if (type == FILE_TYPE_SD_MSG)
    {
        if (sd_msg_total >= SD_RECORD_MAX)
        {
            int i = 0;
            int total = sd_msg_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_msg[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
            }
        }
        file_path = SD_MSG_PATH;
        p_array = &p_sd_msg[sd_msg_total];
        sd_msg_total++;
        sd_msg_new_total++;
    }
    else if (type == FILE_TYPE_SD_MOTION)
    {
        if (sd_motion_total >= SD_RECORD_MAX)
        {
            int i = 0;
            int total = sd_motion_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_motion[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
            }
        }
        file_path = SD_MOTION_PATH;
        p_array = &p_sd_motion[sd_motion_total];
        sd_motion_total++;
        sd_motion_new_total++;
    }
    else if (type == FILE_TYPE_SD_ALARM)
    {
        if (sd_alarm_total >= SD_RECORD_MAX)
        {
            int i = 0;
            int total = sd_alarm_total;
            for (i = 0; i < total; i++)
            {
                if (p_sd_alarm[i].is_lock == false)
                {
                    media_file_delete(type, i);
                    break;
                }
            }
            if (i == total)
            {
                return false;
            }
        }
        file_path = SD_ALARM_PATH;
        p_array = &p_sd_alarm[sd_alarm_total];
        sd_alarm_total++;
        sd_alarm_new_total++;
    }
    else
    {
        Debug_Lib("No this type file.\n\r");
        return false;
    }

    p_array->ch = ch;
    p_array->is_new = 1;
    p_array->mode = mode;
    p_array->type = type;
    p_array->is_lock = 0;
    p_array->file_type = file_type;
    struct ak_date date;
    ak_get_localdate(&date);
    do
    {

        if (file_type == AUDIO_TYPE)
        {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, file_type, MUSIC_DOT);
        }
        else if (file_type == VIDEO_TYPE)
        {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, file_type, VIDEO_DOT);
        }
        else if (file_type == PHOTO_TYPE)
        {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, file_type, PHOTO_DOT);
        }
        else if (file_type == TEXT_TYPE)
        {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%d", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, file_type);
        }

        strcpy(path, file_path);
        strcat(path, p_array->file_name);
        date.second++;
        date.second %= 60;
        Debug_Lib("%s exist\n\r", path);
        ak_sleep_ms(5);
    } while (access(path, F_OK) == 0);

    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_CALL || type == FILE_TYPE_SD_MSG || type == FILE_TYPE_SD_MOTION || type == FILE_TYPE_SD_ALARM || type == FILE_TYPE_FLASH_PHOTO)
    {
        Debug_Lib("----------------FILE_TYPE:%d\n\r ", type);
        sd_file_sync(type);
    }
    return true;
}

media_info *media_info_get(media_type type, int index)
{
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if (index >= sd_mixed_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_mixed[index];
    }
    else if (type == FILE_TYPE_FLASH_PHOTO)
    {
        if (index >= flash_photo_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_flash_photo[index];
    }
    else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {

        if (index >= sd_photo_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_photo[index];
    }
    else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {

        if (index >= sd_video_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_video[index];
    }
    else if ((type == FILE_TYPE_SD_MUSIC) && BG_MIUSIC_FILE_ENABLE)
    {

        if (index >= sd_music_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_music[index];
    }
    else if ((type == FILE_TYPE_SD_PICTURE) && BG_PICTURE_FILE_ENABLE)
    {

        if (index >= sd_picture_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_picture[index];
    }
    else if ((type == FILE_TYPE_SD_CALL) && REC_CALL_FILE_ENABLE)
    {

        if (index >= sd_call_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_call[index];
    }
    else if ((type == FILE_TYPE_SD_MSG) && REC_MSG_FILE_ENABLE)
    {

        if (index >= sd_msg_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_msg[index];
    }
    else if ((type == FILE_TYPE_SD_MOTION) && REC_MOTION_FILE_ENABLE)
    {

        if (index >= sd_motion_total)
        {
            Debug_Lib("get info error %d,%d,%d\n\r", __LINE__, index, sd_motion_total);
            return NULL;
        }
        return &p_sd_motion[index];
    }
    else if ((type == FILE_TYPE_SD_ALARM) && REC_ALARM_FILE_ENABLE)
    {

        if (index >= sd_alarm_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return NULL;
        }
        return &p_sd_alarm[index];
    }
    Debug_Lib("type error ! ,%d\n\r", type);
    return NULL;
}

int media_file_total_get(media_type type, char is_new)
{

    if (type == FILE_TYPE_FLASH_PHOTO)
    {

        return is_new ? flash_photo_new_total : flash_photo_total;
    }
    else
    {

        if (is_sdcard_insert() == 0)
        {

            return 0;
        }
        if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
        {

            return is_new ? sd_mixed_new_total : sd_mixed_total;
        }
        else if (type == FILE_TYPE_SD_PHOTO)
        {

            return is_new ? sd_photo_new_total : sd_photo_total;
        }
        else if (type == FILE_TYPE_SD_VIDEO)
        {

            return is_new ? sd_video_new_total : sd_video_total;
        }
        else if (type == FILE_TYPE_SD_MUSIC)
        {

            return is_new ? sd_music_new_total : sd_music_total;
        }
        else if (type == FILE_TYPE_SD_PICTURE)
        {

            return is_new ? sd_picture_new_total : sd_picture_total;
        }
        else if (type == FILE_TYPE_SD_CALL)
        {

            return is_new ? sd_call_new_total : sd_call_total;
        }
        else if (type == FILE_TYPE_SD_MSG)
        {

            return is_new ? sd_msg_new_total : sd_msg_total;
        }
        else if (type == FILE_TYPE_SD_MOTION)
        {

            return is_new ? sd_motion_new_total : sd_motion_total;
        }
        else if (type == FILE_TYPE_SD_ALARM)
        {

            return is_new ? sd_alarm_new_total : sd_alarm_total;
        }
    }

    Debug_Lib("type error ! \n\r");
    return 0;
}
int media_file_lock_set(media_type type, int index, bool en)
{
    media_info *info = NULL;
    if (type == FILE_TYPE_FLASH_PHOTO)
    {
        if (index >= flash_photo_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_flash_photo[index];
    }
    else if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if (index >= sd_mixed_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_mixed[index];
    }
    else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {
        if (index >= sd_photo_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_photo[index];
    }
    else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {
        if (index >= sd_video_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_video[index];
    }
    else if ((type == FILE_TYPE_SD_MUSIC) && BG_MIUSIC_FILE_ENABLE)
    {
        if (index >= sd_music_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_music[index];
    }
    else if ((type == FILE_TYPE_SD_PICTURE) && BG_MIUSIC_FILE_ENABLE)
    {
        if (index >= sd_picture_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_picture[index];
    }
    else
    {
        Debug_Lib("get info error type \n\r");
        return -1;
    }

    if (info->is_lock == en)
    {
        return 0;
    }

    info->is_lock = en;

    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_FLASH_PHOTO)
    {
        sd_file_sync(type);
    }
    return 0;
}

int media_file_new_clear(media_type type, int index)
{

    media_info *info = NULL;
    // char *path = NULL;

    if (type == FILE_TYPE_FLASH_PHOTO)
    {

        if (index >= flash_photo_total)
        {
            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_flash_photo[index];
        //   path = FLASH_PHOTO_PATH;
        if ((info->is_new) && (flash_photo_new_total > 0))
        {

            flash_photo_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && MIX_PHOTOS_AND_VIDEOS_FILE)
    {

        if (index >= sd_mixed_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_mixed[index];
        //  path = SD_MIXED_PATH;
        if ((info->is_new) && (sd_mixed_new_total > 0))
        {
            sd_mixed_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {

        if (index >= sd_photo_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_photo[index];
        //  path = SD_PHOTO_PATH;
        if ((info->is_new) && (sd_photo_new_total > 0))
        {
            sd_photo_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
    {

        if (index >= sd_video_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_video[index];
        // path = SD_VIDEO_PATH;
        if ((info->is_new) && (sd_video_new_total > 0))
        {
            sd_video_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_MUSIC) && BG_MIUSIC_FILE_ENABLE)
    {

        if (index >= sd_music_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_music[index];
        // path = SD_AUDIO_PATH;
        if ((info->is_new) && (sd_music_new_total > 0))
        {
            sd_music_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_PICTURE) && BG_MIUSIC_FILE_ENABLE)
    {

        if (index >= sd_picture_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_picture[index];
        //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_picture_new_total > 0))
        {
            sd_picture_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_CALL) && REC_CALL_FILE_ENABLE)
    {

        if (index >= sd_call_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_call[index];
        //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_call_new_total > 0))
        {
            sd_call_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_MSG) && REC_MSG_FILE_ENABLE)
    {

        if (index >= sd_msg_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_msg[index];
        //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_msg_new_total > 0))
        {
            sd_msg_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_MOTION) && REC_MOTION_FILE_ENABLE)
    {

        if (index >= sd_motion_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_motion[index];
        //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_motion_new_total > 0))
        {
            sd_motion_new_total--;
        }
    }
    else if ((type == FILE_TYPE_SD_ALARM) && REC_ALARM_FILE_ENABLE)
    {

        if (index >= sd_alarm_total)
        {

            Debug_Lib("get info error %d\n\r", __LINE__);
            return -1;
        }
        info = &p_sd_alarm[index];
        //  path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_alarm_new_total > 0))
        {
            sd_alarm_new_total--;
        }
    }
    else
    {
        Debug_Lib("get info error type \n\r");
        return -1;
    }

    if (info->is_new == 0)
    {

        return 0;
    }

    info->is_new = 0;
#if 0
    char file_path[MEDIA_PATH_MAX] = {0};
    strcat(file_path, path);
    strcat(file_path, info->file_name);

    struct stat st;
    chmod(file_path, S_IRUSR | S_IWUSR);
    stat(file_path, &st);
#endif
    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_FLASH_PHOTO ||
        type == FILE_TYPE_SD_CALL || type == FILE_TYPE_SD_MSG || type == FILE_TYPE_SD_MOTION || type == FILE_TYPE_SD_ALARM)
    {
        sd_file_sync(type);
    }
    return 0;
}

int media_file_delete(media_type type, int index)
{
    media_info *info = NULL, *p_array = NULL;
    char file_path[64];
    int *total_file = NULL, *new_total_file = NULL;

    if (type == FILE_TYPE_FLASH_PHOTO)
    {
        if (index >= flash_photo_total)
            return -1;

        p_array = p_flash_photo;
        info = &p_flash_photo[index];
        strcpy(file_path, FLASH_PHOTO_PATH);
        strcat(file_path, info->file_name);

        total_file = &flash_photo_total;
        new_total_file = &flash_photo_new_total;
    }
    else
    {

        if (is_sdcard_insert() == 0)
        {

            Debug_Lib("no insert sd ,delete fail \n\r");
            return -1;
        }
        if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && (index < sd_mixed_total) && MIX_PHOTOS_AND_VIDEOS_FILE)
        {

            p_array = p_sd_mixed;
            info = &p_sd_mixed[index];
            strcpy(file_path, SD_MIXED_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_mixed_total;
            new_total_file = &sd_mixed_new_total;
        }
        else if ((type == FILE_TYPE_SD_PHOTO && index < sd_photo_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
        {
            p_array = p_sd_photo;
            info = &p_sd_photo[index];
            strcpy(file_path, SD_PHOTO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_photo_total;
            new_total_file = &sd_photo_new_total;
        }
        else if ((type == FILE_TYPE_SD_VIDEO && index < sd_video_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE))
        {

            p_array = p_sd_video;
            info = &p_sd_video[index];
            strcpy(file_path, SD_VIDEO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_video_total;
            new_total_file = &sd_video_new_total;
        }
        else if ((type == FILE_TYPE_SD_MUSIC && index < sd_music_total) && BG_MIUSIC_FILE_ENABLE)
        {

            p_array = p_sd_music;
            info = &p_sd_music[index];
            strcpy(file_path, SD_MUSIC_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_music_total;
            new_total_file = &sd_music_new_total;
        }
        else if ((type == FILE_TYPE_SD_PICTURE && index < sd_picture_total) && BG_PICTURE_FILE_ENABLE)
        {

            p_array = p_sd_picture;
            info = &p_sd_picture[index];
            strcpy(file_path, SD_PICTURE_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_picture_total;
            new_total_file = &sd_picture_new_total;
        }
        else if ((type == FILE_TYPE_SD_CALL && index < sd_call_total) && REC_CALL_FILE_ENABLE)
        {

            p_array = p_sd_call;
            info = &p_sd_call[index];
            strcpy(file_path, SD_CALL_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_call_total;
            new_total_file = &sd_call_new_total;
        }
        else if ((type == FILE_TYPE_SD_MSG && index < sd_msg_total) && REC_MSG_FILE_ENABLE)
        {

            p_array = p_sd_msg;
            info = &p_sd_msg[index];
            strcpy(file_path, SD_MSG_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_msg_total;
            new_total_file = &sd_msg_new_total;
        }
        else if ((type == FILE_TYPE_SD_MOTION && index < sd_motion_total) && REC_MOTION_FILE_ENABLE)
        {

            p_array = p_sd_motion;
            info = &p_sd_motion[index];
            strcpy(file_path, SD_MOTION_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_motion_total;
            new_total_file = &sd_motion_new_total;
        }
        else if ((type == FILE_TYPE_SD_ALARM && index < sd_alarm_total) && REC_ALARM_FILE_ENABLE)
        {

            p_array = p_sd_alarm;
            info = &p_sd_alarm[index];
            strcpy(file_path, SD_ALARM_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_alarm_total;
            new_total_file = &sd_alarm_new_total;
        }
        else
        {
            Debug_Lib("delete media file error.%d\n\r", type);
            return -1;
        }
    }

    if ((*total_file) <= 0)
    {

        Debug_Lib("delete fail file total %d \n\r", *total_file);
        return -1;
    }
    if ((info->is_new) && ((*new_total_file) > 0))
    {

        (*new_total_file)--;
    }

    remove(file_path);
    Debug_Lib("del %d.%s \n\r", index, file_path);

    if (index < ((*total_file) - 1))
    {

        memmove(&p_array[index], &p_array[index + 1], (((*total_file) - 1) - index) * sizeof(media_info));
    }
    (*total_file)--;

    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_SD_CALL || type == FILE_TYPE_SD_MSG || type == FILE_TYPE_SD_MOTION || type == FILE_TYPE_SD_ALARM || type == FILE_TYPE_FLASH_PHOTO)
    {
        sd_file_sync(type);
    }
    return 0;
}

int record_null_error_file_remove(const char *file, bool is_null)
{
    if (is_sdcard_insert() == 0)
    {
        Debug_Lib("no insert sd\n\r");
        return -1;
    }

    char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file)))
    {

        remove(bad_file);
        int index = sd_video_total - 1;
        media_file_delete(p_sd_video[index].type, index);
        return 1;
    }
    else if (is_null)
    {
        int index = sd_video_total - 1;
        media_file_delete(p_sd_video[index].type, index);
        return 1;
    }
    return 0;
}

int snap_null_error_file_remove(const char *file, bool is_null)
{
    char bad_file[MEDIA_PATH_MAX] = {0};
    if (_photo_bad_path_check(file, bad_file))
    {

        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
        {
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);

            remove(bad_file);
            return 1;
        }
        else
        {

            int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);

            remove(bad_file);
            return 1;
        }
    }
    else if (is_null)
    {
        Debug_Lib("+++++%s \n\r", file);
        if (is_sdcard_insert() == 0)
        {
            int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);
            return 1;
        }
        else if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
        {
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);
            return 1;
        }
    }
    return 0;
}

int playback_bad_file_check(const char *file, int index)
{
    char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file)))
    {

        remove(bad_file);
        media_file_delete(p_sd_video[index].type, index);

        return 1;
    }
    else if (_photo_bad_path_check(file, bad_file))
    {

        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
        {

            remove(bad_file);
            media_file_delete(p_sd_photo[index].type, index);
            return 1;
        }
        else
        {

            remove(bad_file);
            media_file_delete(p_flash_photo[index].type, index);
            return 1;
        }
    }
    return 0;
}

bool Media_bad_path_check(const char *file, int index)
{
    if (is_sdcard_insert() == 0)
    {
        Debug_Lib("no insert sd\n\r");
        return false;
    }
    int del = -1;
    char bad_file[MEDIA_PATH_MAX] = {0};
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if ((_video_bad_path_check(file, bad_file)))
        {
            remove(bad_file);
            del = media_file_delete(p_sd_mixed[index].type, index);
        }
        else if (_photo_bad_path_check(file, bad_file))
        {
            if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
            {
                del = media_file_delete(p_sd_mixed[index].type, index);
            }
            else
            {
                del = media_file_delete(p_flash_photo[index].type, index);
            }
        }
    }
    else
    {

        if ((_video_bad_path_check(file, bad_file)))
        {
            remove(bad_file);
            del = media_file_delete(p_sd_video[index].type, index);
        }
        else if (_photo_bad_path_check(file, bad_file))
        {
            if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
            {
                del = media_file_delete(p_sd_photo[index].type, index);
            }
            else
            {
                del = media_file_delete(p_flash_photo[index].type, index);
            }
        }
    }
    return (del == -1) ? false : true;
}

int media_bad_path_check(const char *file, char mode)
{
    Debug_Lib("%s =====================>>%d\n\r", __func__, mode);
    if (is_sdcard_insert() == 0)
    {
        Debug_Lib("no insert sd\n\r");
        return -1;
    }

    char bad_file[MEDIA_PATH_MAX] = {0};
    if (MIX_PHOTOS_AND_VIDEOS_FILE)
    {
        if ((_video_bad_path_check(file, bad_file)))
        {
            remove(bad_file);
            int index = sd_mixed_total - 1;
            media_file_delete(p_sd_mixed[index].type, index);
        }
        else if (_photo_bad_path_check(file, bad_file))
        {
            if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
            {
                int index = sd_mixed_total - 1;
                media_file_delete(p_sd_mixed[index].type, index);
            }
            else
            {
                int index = flash_photo_total - 1;
                media_file_delete(p_flash_photo[index].type, index);
            }
        }
    }
    else
    {

        int index;
        media_info *P = NULL;
        switch (mode)
        {
        case FILE_TYPE_SD_CALL:
            index = sd_call_total - 1;
            P = p_sd_call;
            break;
        case FILE_TYPE_SD_MSG:
            index = sd_msg_total - 1;
            P = p_sd_msg;
            break;
        case FILE_TYPE_SD_MOTION:
            index = sd_motion_total - 1;
            P = p_sd_motion;
            break;
        case FILE_TYPE_SD_ALARM:
            index = sd_alarm_total - 1;
            P = p_sd_alarm;
            break;

        default:
            break;
        }

        if (P == NULL)
        {
            if ((_video_bad_path_check(file, bad_file)))
            {
                remove(bad_file);
                int index = sd_video_total - 1;
                media_file_delete(p_sd_video[index].type, index);
            }
            else if (_photo_bad_path_check(file, bad_file))
            {
                if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
                {
                    int index = sd_photo_total - 1;
                    media_file_delete(p_sd_photo[index].type, index);
                }
                else
                {
                    int index = flash_photo_total - 1;
                    media_file_delete(p_flash_photo[index].type, index);
                }
            }
        }
        else
        {
            if ((_video_bad_path_check(file, bad_file)) || _photo_bad_path_check(file, bad_file))
            {
                remove(bad_file);
                media_file_delete(P[index].type, index);
            }
        }
    }

    return 0;
}

/***********************************************
** 作者: leo.liu
** 日期: 2022-11-5 17:11:35
** 说明: 查看sd卡信息
***********************************************/
bool tuya_sd_memory_query(unsigned int *p_total, unsigned int *p_user, unsigned int *p_free)
{
    if (is_sdcard_insert() == false)
    {
        *p_total = *p_user = *p_free = 0;
        return false;
    }

    unsigned long long total_size_kb, free_size_kb, used_size_kb;

#ifndef SDCARD_PARTITION
    if (0 != statfs(SD_BASE_PATH, &diskinfo_tuya))
    {
        return false;
    }
#else
    if (0 != statfs(SD_BASE_PATH2, &diskinfo_tuya))
    {
        return false;
    }
#endif
    total_size_kb = (unsigned long long)diskinfo_tuya.f_blocks * diskinfo_tuya.f_bsize / 1024;
    free_size_kb = (unsigned long long)diskinfo_tuya.f_bfree * diskinfo_tuya.f_bsize / 1024;
    used_size_kb = total_size_kb - free_size_kb;
    // Debug_Lib("%s total_size_kb:%llu  free_size_kb:%llu user_size_kb:%llu \n",__func__,total_size_kb,free_size_kb,used_size_kb);
    *p_user = (unsigned int)(used_size_kb /* /1024 */);
    *p_total = (unsigned int)(total_size_kb /* /1024 */);
    *p_free = (unsigned int)(free_size_kb /* /1024 */);
    return true;
}

static void copy_file(const char *source, const char *destination)
{
    FILE *src, *dest;
    src = fopen(source, "rb");
    if (src == NULL)
    {
        Debug_Lib("无法打开源文件 %s\n", source);
        return;
    }
    dest = fopen(destination, "wb");
    if (dest == NULL)
    {
        Debug_Lib("无法创建目标文件 %s\n", destination);
        fclose(src);
        return;
    }
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dest);
    }
    Debug_Lib("成功复制文件 %s 至 %s\n", source, destination);
    fclose(src);
    fclose(dest);
}

static void sdcard_upgrade_package_copy(char *src_dir, char *dst_dir, bool rm_backup)
{
    extern unsigned long long os_get_ms(void);
    unsigned long long x = os_get_ms();

    char src_path[128] = {0};
    char dst_path[128] = {0};
    sprintf(src_path, "%s%s", src_dir, UPGRADE_OLD_PACKAGR_PATH);
    sprintf(dst_path, "%s%s", dst_dir, UPGRADE_OLD_PACKAGR_PATH);
    if (access(src_path, F_OK) == 0)
    {
        Debug_Lib("%s exist!!!\n", src_path);
        if (access(dst_path, F_OK) != 0)
        {
            Debug_Lib("%s no exist!!!  backup", dst_path);
            copy_file(src_path, dst_path);

            if (rm_backup)
                remove(src_path);
        }
        else
        {
            Debug_Lib("%s exist!!! no backup\n", dst_path);
        }
    }

    DIR *src;
    DIR *dst;
    struct dirent *entry;
    char source[256], destination[256];
    src = opendir(src_dir);
    if (src == NULL)
    {
        Debug_Lib("无法打开目录 %s\n", src_dir);
        return;
    }
    dst = opendir(dst_dir);
    if (dst == NULL)
    {
        Debug_Lib("无法打开目录 %s\n", dst_dir);
        closedir(src);
        return;
    }

    while ((entry = readdir(src)) != NULL)
    {
        if (strncmp(entry->d_name, UPGRADE_NEW_PACKAGR_PATH, strlen(UPGRADE_NEW_PACKAGR_PATH)) == 0)
        {
            Debug_Lib("%s%s* exist!!!  backup", src_dir, UPGRADE_NEW_PACKAGR_PATH);
            snprintf(source, sizeof(source), "%s%s", src_dir, entry->d_name);
            snprintf(destination, sizeof(destination), "%s%s", dst_dir, entry->d_name);
            copy_file(source, destination);

            if (rm_backup)
                remove(source);

            system("sync");
            goto exit;
        }
    }

    Debug_Lib("%s%s* no exist!!!  backup", src_dir, UPGRADE_NEW_PACKAGR_PATH);

exit:
    closedir(dst);
    closedir(src);
    Debug_Lib("%llu elapsed\n", os_get_ms() - x);
}
