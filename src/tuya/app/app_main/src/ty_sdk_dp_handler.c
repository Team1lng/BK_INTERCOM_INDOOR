/*********************************************************************************
  *Copyright(C),2015-2020, 
  *TUYA 
  *www.tuya.comm

  *FileName: tuya_ipc_dp_handler.c
  *
  * File Description：
  * 1. DP Point Setting and Acquisition Function API
  *
  * Developer work：
  * 1. Local configuration acquisition and update.
  * 2. Set local IPC attributes, such as picture flip, time watermarking, etc.
  *    If the function is not supported, leave the function blank.
  *
**********************************************************************************/
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_ptz.h"
#include "ty_dp_define.h"
#include "ty_sdk_common.h"

#include "tkl_wifi.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

extern CHAR_T s_ipc_storage[64];

STATIC VOID __tuya_json_write(CHAR_T *buffer)
{
    CHAR_T config_path[256] = {0};
    snprintf(config_path, 256, "%s/tuya_dp_cfg.json", s_ipc_storage);

    FILE *fp = fopen(config_path, "w");
    fwrite(buffer, strlen(buffer), 1, fp); 
    fclose(fp);
}

VOID TUYA_IPC_INIT_DP_CONFIG()
{
    CHAR_T config_path[256] = {0};
    INT_T fd = -1;
    INT_T sleep_mode = 0;
    INT_T light_switch = 0;
    INT_T flip_switch = 0;
    INT_T watermark_switch = 0;
    INT_T wdr_switch = 0;
    INT_T alarm_switch = 0;
    INT_T alarm_zone_switch = 0;
    INT_T sd_record_mode = 0;
    INT_T sd_record_switch = 0;
    INT_T blub_switch = 0;
    INT_T ap_switch = 0;
    CHAR_T night_mode[8] = {0};
    CHAR_T alarm_sen[8] = {0};
    CHAR_T ap_ssid[32] = {0};
    CHAR_T ap_passwd[32] = {0};

    snprintf(config_path, 256, "%s/tuya_dp_cfg.json", s_ipc_storage);
    if(0 == access(config_path, F_OK) ){
        return;
    }else{
        fd = open(config_path, O_RDWR|O_CREAT,0666);
        if (fd < 0 ){
            PR_ERR("create %s failed \n", config_path);
            return;
        }
        ty_cJSON *cfg_json = ty_cJSON_CreateObject();
        ty_cJSON_AddItemToObject(cfg_json, "tuya_sleep_mode", ty_cJSON_CreateNumber(sleep_mode));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_light_onoff", ty_cJSON_CreateNumber(light_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_flip_onoff", ty_cJSON_CreateNumber(flip_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_watermark_onoff", ty_cJSON_CreateNumber(watermark_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_wdr_onoff", ty_cJSON_CreateNumber(wdr_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_alarm_on_off", ty_cJSON_CreateNumber(alarm_switch));
        ty_cJSON_AddItemToObject(cfg_json, "alarm_zone_on_off", ty_cJSON_CreateNumber(alarm_zone_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_sd_record_mode", ty_cJSON_CreateNumber(sd_record_mode));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_sd_record_on_off", ty_cJSON_CreateNumber(sd_record_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_blub_on_off", ty_cJSON_CreateNumber(blub_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_on_off", ty_cJSON_CreateNumber(ap_switch));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_night_mode", ty_cJSON_CreateString(night_mode));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_alarm_sen", ty_cJSON_CreateString(alarm_sen));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_ssid", ty_cJSON_CreateString(ap_ssid));
        ty_cJSON_AddItemToObject(cfg_json, "tuya_ap_passwd", ty_cJSON_CreateString(ap_passwd));

        CHAR_T *buffer = ty_cJSON_Print(cfg_json);
        __tuya_json_write(buffer);
        ty_cJSON_free(buffer);
        ty_cJSON_Delete(cfg_json);
    }
}

STATIC ty_cJSON *__tuya_json_parse()
{
    CHAR_T buffer[4096] = {0};
    INT_T read_len = sizeof(buffer);
    INT_T fd = -1;
    CHAR_T config_path[256] = {0};

    snprintf(config_path, 256, "%s/tuya_dp_cfg.json", s_ipc_storage);
    fd = open(config_path, O_RDWR);
    if (fd < 0 ){
        PR_ERR("create %s failed \n", config_path);
        return NULL;
    }

    while (read_len == read(fd, buffer, sizeof(buffer)));
    close(fd);

    ty_cJSON *cfg_json = ty_cJSON_Parse(buffer);
    return cfg_json;
}


/* Setting and Getting Integer Local Configuration.
The reference code here is written directly to tmp, requiring developers to replace the key.*/
STATIC VOID __tuya_app_write_INT(CHAR_T *key, INT_T value)
{
    ty_cJSON *cfg_json = __tuya_json_parse();
    if (NULL == cfg_json){
        PR_ERR("load configure failed\n");
        return;
    }
    ty_cJSON_ReplaceItemInObject(cfg_json, key, ty_cJSON_CreateNumber(value));
    CHAR_T *json_tmp = ty_cJSON_Print(cfg_json);
    __tuya_json_write(json_tmp);
    ty_cJSON_free(json_tmp);
    ty_cJSON_Delete(cfg_json);
}

STATIC INT_T __tuya_app_read_INT(CHAR_T *key)
{
    int tmp_val = 0;
    ty_cJSON *cfg_json = __tuya_json_parse();
    if (NULL == cfg_json){
        PR_ERR("load configure failed\n");
        return OPRT_CJSON_PARSE_ERR;
    }

    ty_cJSON* val = ty_cJSON_GetObjectItem(cfg_json, key);
    if (val) {
        tmp_val = val->valueint;
    }
    ty_cJSON_Delete(cfg_json);
    return tmp_val;
}

/* Setting and Getting String Local Configuration.
The reference code here is written directly to tmp, requiring developers to replace the key.*/
STATIC VOID __tuya_app_write_STR(CHAR_T *key, CHAR_T *value)
{
    ty_cJSON *cfg_json = __tuya_json_parse();
    if (NULL == cfg_json){
        PR_ERR("load configure failed\n");
        return;
    }
    ty_cJSON_ReplaceItemInObject(cfg_json, key, ty_cJSON_CreateString(value));
    CHAR_T *json_tmp = ty_cJSON_Print(cfg_json);
    __tuya_json_write(json_tmp);
    ty_cJSON_free(json_tmp);
    ty_cJSON_Delete(cfg_json);
}

STATIC INT_T __tuya_app_read_STR(CHAR_T *key, CHAR_T *value, INT_T value_size)
{
    memset(value, 0, value_size);
    ty_cJSON *cfg_json = __tuya_json_parse();
    if (NULL == cfg_json){
        PR_ERR("load configure failed\n");
        return OPRT_CJSON_PARSE_ERR;
    }
    ty_cJSON* val = ty_cJSON_GetObjectItem(cfg_json, key);
    if (val) {
        strncpy(value, val->valuestring, value_size);
    }
    ty_cJSON_Delete(cfg_json);
    return 0;

}

#ifdef TUYA_DP_SLEEP_MODE
VOID IPC_APP_set_sleep_mode(BOOL_T sleep_mode)
{
    PR_INFO("set sleep_mode:%d \r\n", sleep_mode);
    //TODO
    /* sleep mode,BOOL type,true means sleep,false means wake */

    __tuya_app_write_INT("tuya_sleep_mode", sleep_mode);
}

BOOL_T IPC_APP_get_sleep_mode(VOID)
{
    BOOL_T sleep_mode = __tuya_app_read_INT("tuya_sleep_mode");
    PR_INFO("curr sleep_mode:%d \r\n", sleep_mode);
    return sleep_mode;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_LIGHT
VOID IPC_APP_set_light_onoff(BOOL_T light_on_off)
{
    PR_INFO("set light_on_off:%d \r\n", light_on_off);
    //TODO
    /* Status indicator,BOOL type,true means on,false means off */

    __tuya_app_write_INT("tuya_light_onoff", light_on_off);
}

BOOL_T IPC_APP_get_light_onoff(VOID)
{
    BOOL_T light_on_off = __tuya_app_read_INT("tuya_light_onoff");
    PR_INFO("curr light_on_off:%d \r\n", light_on_off);
    return light_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_FLIP
VOID IPC_APP_set_flip_onoff(BOOL_T flip_on_off)
{
    PR_INFO("set flip_on_off:%d \r\n", flip_on_off);
    //TODO
    /* flip state,BOOL type,true means inverse,false means normal */

    __tuya_app_write_INT("tuya_flip_onoff", flip_on_off);
}

BOOL_T IPC_APP_get_flip_onoff(VOID)
{
    BOOL_T flip_on_off = __tuya_app_read_INT("tuya_flip_onoff");
    PR_INFO("curr flip_on_off:%d \r\n", flip_on_off);
    return flip_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_WATERMARK
VOID IPC_APP_set_watermark_onoff(BOOL_T watermark_on_off)
{
    PR_INFO("set watermark_on_off:%d \r\n", watermark_on_off);
    //TODO
    /* Video watermarking (OSD),BOOL type,true means adding watermark on,false means not */

    __tuya_app_write_INT("tuya_watermark_onoff", watermark_on_off);
}

BOOL_T IPC_APP_get_watermark_onoff(VOID)
{
    BOOL_T watermark_on_off = __tuya_app_read_INT("tuya_watermark_onoff");
    PR_INFO("curr watermark_on_off:%d \r\n", watermark_on_off);
    return watermark_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_WDR
VOID IPC_APP_set_wdr_onoff(BOOL_T wdr_on_off)
{
    PR_INFO("set wdr_on_off:%d \r\n", wdr_on_off);
    //TODO
    /* Wide Dynamic Range Model,BOOL type,true means on,false means off */

    __tuya_app_write_INT("tuya_wdr_onoff", wdr_on_off);

}

BOOL_T IPC_APP_get_wdr_onoff(VOID)
{
    BOOL_T wdr_on_off = __tuya_app_read_INT("tuya_wdr_onoff");
    PR_INFO("curr watermark_on_off:%d \r\n", wdr_on_off);
    return wdr_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_NIGHT_MODE
STATIC CHAR_T s_night_mode[4] = {0};//for demo
VOID IPC_APP_set_night_mode(CHAR_T *p_night_mode)
{//0-automatic 1-off 2-on
    PR_INFO("set night_mode:%s \r\n", p_night_mode);
    //TODO
    /* Infrared night vision function,ENUM type*/

    __tuya_app_write_STR("tuya_night_mode", p_night_mode);
}

CHAR_T *IPC_APP_get_night_mode(VOID)
{
    __tuya_app_read_STR("tuya_night_mode", s_night_mode, 4);
    PR_INFO("curr IPC_APP_get_night_mode:%s  \r\n", s_night_mode );
    return  s_night_mode;
}

#endif


//------------------------------------------

#ifdef TUYA_DP_ALARM_FUNCTION
VOID IPC_APP_set_alarm_function_onoff(BOOL_T alarm_on_off)
{
    PR_INFO("set alarm_on_off:%d \r\n", alarm_on_off);
    /* motion detection alarm switch,BOOL type,true means on,false means off.
     * This feature has been implemented, and developers can make local configuration settings and properties.*/

    __tuya_app_write_INT("tuya_alarm_on_off", alarm_on_off);
}

BOOL_T IPC_APP_get_alarm_function_onoff(VOID)
{
    BOOL_T alarm_on_off = __tuya_app_read_INT("tuya_alarm_on_off");
    PR_INFO("curr alarm_on_off:%d \r\n", alarm_on_off);
    return alarm_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ALARM_SENSITIVITY
STATIC CHAR_T s_alarm_sensitivity[4] = {0};//for demo
VOID IPC_APP_set_alarm_sensitivity(CHAR_T *p_sensitivity)
{
    PR_INFO("set alarm_sensitivity:%s \r\n", p_sensitivity);
    //TODO
    /* Motion detection alarm sensitivity,ENUM type,0 means low sensitivity,1 means medium sensitivity,2 means high sensitivity*/

    __tuya_app_write_STR("tuya_alarm_sen", p_sensitivity);
}

CHAR_T *IPC_APP_get_alarm_sensitivity(VOID)
{
    __tuya_app_read_STR("tuya_alarm_sen", s_alarm_sensitivity, 4);
    PR_INFO("curr alarm_sensitivity:%s \r\n", s_alarm_sensitivity);
    return s_alarm_sensitivity;
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
VOID IPC_APP_set_alarm_zone_onoff(BOOL_T alarm_zone_on_off)
{
    /* Motion detection area setting switch,BOOL type,true means on,false is off*/
    PR_INFO("set alarm_zone_onoff:%d \r\n", alarm_zone_on_off);
  
    /* This feature has been implemented, and developers can make local configuration settings and properties.*/

    __tuya_app_write_INT("alarm_zone_on_off", alarm_zone_on_off);

}

BOOL_T IPC_APP_get_alarm_zone_onoff(VOID)
{
    BOOL_T alarm_zone_on_off = __tuya_app_read_INT("alarm_zone_on_off");
    PR_INFO("curr alarm_on_off:%d \r\n", alarm_zone_on_off);
    return alarm_zone_on_off;
}
#endif


#ifdef TUYA_DP_ALARM_ZONE_DRAW

#define MAX_ALARM_ZONE_NUM      (6)     //Supports the maximum number of detection areas
//Detection area structure
typedef struct{
    char pointX;    //Starting point x  [0-100]
    char pointY;    //Starting point Y  [0-100]
    char width;     //width    [0-100]
    char height;    //height    [0-100]
}ALARM_ZONE_T;

typedef struct{
    int iZoneNum;   //Number of detection areas
    ALARM_ZONE_T alarmZone[MAX_ALARM_ZONE_NUM];
}ALARM_ZONE_INFO_T;


VOID IPC_APP_set_alarm_zone_draw(cJSON * p_alarm_zone)
{
    if (NULL == p_alarm_zone){
        return ;
    }
    #if 0
    /*demo code*/
    /*Motion detection area setting switch*/
    PR_INFO("%s %d set alarm_zone_set:%s \r\n",__FUNCTION__,__LINE__, (char *)p_alarm_zone);
    ALARM_ZONE_INFO_T strAlarmZoneInfo;
    INT_T i = 0;
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_alarm_zone);

    if (NULL == pJson){
        PR_INFO("%s %d step error\n",__FUNCTION__,__LINE__);
        //free(pResult);
        return;
    }
    cJSON * tmp = cJSON_GetObjectItem(pJson, "num");
    if (NULL == tmp){
        PR_INFO("%s %d step error\n",__FUNCTION__,__LINE__);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
    strAlarmZoneInfo.iZoneNum = tmp->valueint;
    PR_INFO("%s %d step num[%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.iZoneNum);
    if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM){
        PR_INFO("#####error zone num too big[%d]\n",strAlarmZoneInfo.iZoneNum);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++){
        char region[12] = {0};
        cJSON * cJSONRegion = NULL;
        snprintf(region, 12, "region%d",i);
        cJSONRegion = cJSON_GetObjectItem(pJson, region);
        if (NULL == cJSONRegion){
            PR_INFO("#####[%s][%d]error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            //free(pResult);
            return;
        }
        strAlarmZoneInfo.alarmZone[i].pointX = cJSON_GetObjectItem(cJSONRegion, "x")->valueint;
        strAlarmZoneInfo.alarmZone[i].pointY = cJSON_GetObjectItem(cJSONRegion, "y")->valueint;
        strAlarmZoneInfo.alarmZone[i].width = cJSON_GetObjectItem(cJSONRegion,  "xlen")->valueint;
        strAlarmZoneInfo.alarmZone[i].height = cJSON_GetObjectItem(cJSONRegion, "ylen")->valueint;
        PR_INFO("#####[%s][%d][%d,%d,%d,%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.alarmZone[i].pointX,\
            strAlarmZoneInfo.alarmZone[i].pointY,strAlarmZoneInfo.alarmZone[i].width,strAlarmZoneInfo.alarmZone[i].height);
    }
    cJSON_Delete(pJson);
    //free(pResult);
    #endif
    return ;
}

VOID IPC_APP_get_alarm_zone_draw(char * p_alarm_zone)
{
    /*demo code*/
    int i;
    ALARM_ZONE_INFO_T strAlarmZoneInfo;

    memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
    strAlarmZoneInfo.iZoneNum = 1;
    strAlarmZoneInfo.alarmZone[0].pointX = 0;
    strAlarmZoneInfo.alarmZone[0].pointY = 0;
    strAlarmZoneInfo.alarmZone[0].width = 100;
    strAlarmZoneInfo.alarmZone[0].height = 100;
    /*get param of alarmzoneInfo yourself*/
    if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM){
        PR_ERR("[%s] [%d ]get iZoneNum[%d] error",__FUNCTION__,__LINE__,strAlarmZoneInfo.iZoneNum);
        return ;
    }
    for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++){
        char region[64] = {0};
        //{"169":"{\"num\":1,\"region0\":{\"x\":0,\"y\":0,\"xlen\":50,\"ylen\":50}}"}
        if (0 == i){
            snprintf(p_alarm_zone, 256,"{\"num\":%d",strAlarmZoneInfo.iZoneNum);
        }
        snprintf(region, 64, ",\"region%d\":{\"x\":%d,\"y\":%d,\"xlen\":%d,\"ylen\":%d}",i,strAlarmZoneInfo.alarmZone[i].pointX,\
            strAlarmZoneInfo.alarmZone[i].pointY,strAlarmZoneInfo.alarmZone[i].width,strAlarmZoneInfo.alarmZone[i].height);
        strcat(p_alarm_zone, region);
        if(i == (strAlarmZoneInfo.iZoneNum - 1)){
            strcat(p_alarm_zone, "}");
        }
    }
    return ;
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_status(VOID)
{
    // INT_T sd_status = 1;
    // /* SD card status, VALUE type, 1-normal, 2-anomaly, 3-insufficient space, 4-formatting, 5-no SD card */
    // /* Developer needs to return local SD card status */
    // //TODO
    // sd_status = 1;

    // PR_INFO("curr sd_status:%d \r\n", sd_status);
    return (INT_T)tuya_ipc_sd_get_status();
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_get_sd_storage(UINT_T *p_total, UINT_T *p_used, UINT_T *p_empty)
{//unit is kb
    //TODO
    /* Developer needs to return local SD card status */
    *p_total = 128 * 1000 * 1000;
    *p_used = 32 * 1000 * 1000;
    *p_empty = *p_total - *p_used;

    PR_INFO("curr sd total:%u used:%u empty:%u \r\n", *p_total, *p_used, *p_empty);
}
#endif
//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_MODE
VOID IPC_APP_set_sd_record_mode(UINT_T sd_record_mode)
{
    PR_INFO("set sd_record_mode:%d \r\n", sd_record_mode);
    if(0 == sd_record_mode)
    {
         tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);
    }
    else if(1 == sd_record_mode)
    {
         tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
    }
    else
    {
        PR_INFO("Error, should not happen\r\n");
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
    }
    __tuya_app_write_INT("tuya_sd_record_mode", sd_record_mode);
}

UINT_T IPC_APP_get_sd_record_mode(VOID)
{
    BOOL_T sd_record_mode = __tuya_app_read_INT("tuya_sd_record_mode");
    PR_TRACE("curr sd_record_mode:%d \r\n", sd_record_mode);
    return sd_record_mode;
}
#endif
//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_ENABLE
BOOL_T IPC_APP_get_sd_record_onoff(VOID)
{
    BOOL_T sd_record_on_off  = __tuya_app_read_INT("tuya_sd_record_on_off");
    PR_TRACE("curr sd_record_on_off:%d \r\n", sd_record_on_off);
    return sd_record_on_off;
}
VOID IPC_APP_set_sd_record_onoff(BOOL_T sd_record_on_off)
{
    PR_INFO("set sd_record_on_off:%d \r\n", sd_record_on_off);
    /* SD card recording function swith, BOOL type, true means on, false means off.
     * This function has been implemented, and developers can make local configuration settings and properties.*/

    if(sd_record_on_off == TRUE)
    {
         IPC_APP_set_sd_record_mode( IPC_APP_get_sd_record_mode()  );
    }else
    {
        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
    }

    __tuya_app_write_INT("tuya_sd_record_on_off", sd_record_on_off);
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_UMOUNT
BOOL_T IPC_APP_unmount_sd_card(VOID)
{
    BOOL_T umount_ok = TRUE;

    //TODO
    /* unmount sdcard */

    PR_INFO("unmount result:%d \r\n", umount_ok);
    return umount_ok;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_FORMAT
/* -2000: SD card is being formatted, -2001: SD card formatting is abnormal, -2002: No SD card, 
   -2003: SD card error. Positive number is formatting progress */
STATIC INT_T s_sd_format_progress = 0;
void *thread_sd_format(void *arg)
{
    /* First notify to app, progress 0% */

    s_sd_format_progress = 0;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tal_system_sleep(100);

    /* Stop local SD card recording and playback, progress 10%*/
    s_sd_format_progress = 10;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
    tuya_ipc_ss_pb_stop_all();
    tal_system_sleep(100);

    /* Delete the media files in the SD card, the progress is 30% */
    s_sd_format_progress = 30;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    // tuya_ipc_ss_delete_all_files();
    tal_system_sleep(100);

    /* Perform SD card formatting operation */
    start_format_sd_card(2);
    while (format_sd_card_status() && is_sdcard_insert())
    {
        tal_system_sleep(100);
    }
    s_sd_format_progress = 80;
    IPC_APP_report_sd_format_status(s_sd_format_progress);
    // TODO
    //     IPC_APP_set_sd_record_onoff( IPC_APP_get_sd_record_onoff());

    /* progress 100% */
    tal_system_sleep(100);
    s_sd_format_progress = 100;
    IPC_APP_report_sd_format_status(s_sd_format_progress);

    tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);

    // handle_DP_SD_STORAGE_ONLY_GET(NULL);
    printf("format sd_card finish!!!!\r\n");

    pthread_exit(0);
    return NULL;
}

VOID IPC_APP_format_sd_card(VOID)
{
    PR_INFO("start to format sd_card \r\n");
    /* SD card formatting.
     * The SDK has already completed the writing of some of the code, 
     and the developer only needs to implement the formatting operation. */

    pthread_t sd_format_thread;
    pthread_create(&sd_format_thread, NULL, thread_sd_format, NULL);
    pthread_detach(sd_format_thread);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_format_status(VOID)
{
    return s_sd_format_progress;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_PTZ_CONTROL
VOID IPC_APP_ptz_start_move(CHAR_T *p_direction)
{
    PR_INFO("ptz start move:%s \r\n", p_direction);
    //0-up, 1-upper right, 2-right, 3-lower right, 4-down, 5-down left, 6-left, 7-top left

}
#endif

#ifdef TUYA_DP_PTZ_STOP
VOID IPC_APP_ptz_stop_move(VOID)
{
    PR_INFO("ptz stop move \r\n");
    //TODO
    /* PTZ rotation stops */

}
#endif

#ifdef TUYA_DP_PTZ_CHECK
void IPC_APP_ptz_check(VOID)
{
    PR_INFO("ptz check \r\n");
}
#endif

#ifdef TUYA_DP_TRACK_ENABLE
void IPC_APP_track_enable(BOOL_T track_enable)
{
    PR_INFO("track_enable %d\r\n",track_enable);
}

BOOL_T IPC_APP_get_track_enable(void)
{
    char track_enable = 0;
    //the value you get yourself
    return (BOOL_T)track_enable;
}

#endif

#ifdef TUYA_DP_HUM_FILTER
void IPC_APP_human_filter(BOOL_T filter_enable)
{
    PR_INFO("filter_enable %d\r\n",filter_enable);
    return;
}
#endif

#ifdef TUYA_DP_PATROL_MODE
void IPC_APP_set_patrol_mode(BOOL_T patrol_mode)
{
    PR_INFO("patrol_mode %d\r\n",patrol_mode);
    return;
}

char IPC_APP_get_patrol_mode(void)
{
    char patrol_mode = 0;
    //the value you get yourself
    return patrol_mode;
}

#endif

#ifdef TUYA_DP_PATROL_SWITCH
void IPC_APP_set_patrol_switch(BOOL_T patrol_switch)
{
    PR_INFO("patrol_switch %d\r\n",patrol_switch);
    return;
}

BOOL_T IPC_APP_get_patrol_switch(void)
{
    char patrol_switch = 0;
    //the value you get yourself
    return (BOOL_T)patrol_switch;
}

void IPC_APP_ptz_preset_reset(S_PRESET_CFG_T *preset_cfg)
{
    /*Synchronize data from server*/
    return;
}

#endif

#ifdef TUYA_DP_PATROL_TMODE
void IPC_APP_set_patrol_tmode(BOOL_T patrol_tmode)
{
    PR_INFO("patrol_tmode %d\r\n",patrol_tmode);
    return;
}

char IPC_APP_get_patrol_tmode(void)
{
    char patrol_tmode = 0;
    //the value you get yourself
    return patrol_tmode;
}

#endif

#ifdef TUYA_DP_PATROL_TIME
void IPC_APP_set_patrol_time(cJSON * p_patrol_time)
{
    //set your patrol_time
    /*

    cJSON * pJson = cJSON_Parse((CHAR_T *)p_patrol_time);
    if (NULL == pJson){
        TYWARN("----error---\n");

        return -1;
    }
    cJSON* t_start = cJSON_GetObjectItem(pJson, "t_start");
    cJSON* t_end = cJSON_GetObjectItem(pJson, "t_end");
    if ((NULL == t_start) || (NULL == t_end)){
        TYWARN("----t_start---\n");
        cJSON_Delete(pJson);
        return -1;
    }
    PR_DEBUG("stare%s--end:%s\n", t_start->valuestring,t_end->valuestring);

    */
    return;
}

#endif

#ifdef TUYA_DP_PRESET_SET
void IPC_APP_set_preset(cJSON * p_preset_param)
{
    //preset add ,preset del, preset go
#if 0
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_preset_param);
    if (NULL == pJson){
        PR_ERR("null preset set input");
        return -1;
    }
    cJSON* type = cJSON_GetObjectItem(pJson, "type");
    cJSON* data = cJSON_GetObjectItem(pJson, "data");
    if ((NULL == type) || (NULL == data)){
        PR_ERR("invalid preset set input");
        return -1;
    }
   

    PR_DEBUG("preset set type: %d",type->valueint);
    //1:add preset point 2:delete preset point 3:call preset point
    if(type->valueint == 1)
    {
        char pic_buffer[PRESET_PIC_MAX_SIZE] = {0};  
        int pic_size = sizeof(pic_buffer);  /*Image to be shown*/
        S_PRESET_POSITION preset_pos;
        char respond_add[128] = {0}; 
        /*mpId is 1,2,3,4,5,6，The server will generate a set of its own preset point number information based on the mpid.*/
        preset_pos.mpId = 1;   
        preset_pos.ptz.pan = 100; /*horizontal position*/
        preset_pos.ptz.tilt = 60;/*vertical position*/
        cJSON* name = cJSON_GetObjectItem(data, "name");
        int name_len = 0;
        int error_num = 0;
        
        if(name == NULL)
        {
            PR_ERR("name is null\n");
            return -1;
        }
        name_len = strlen(name->valuestring);
        name_len = (name_len > 30)?30:name_len;
        memcpy(&preset_pos.name,name->valuestring,(name_len));
        preset_pos.name[name_len] = '\0';
        error_num = tuya_ipc_preset_add(&preset_pos);

        snprintf(respond_add,128,"{\"type\":%d,\"data\":{\"error\":%d}}",type->valueint,error_num);

        tuya_ipc_dp_report(NULL, TUYA_DP_PRESET_SET,PROP_STR,respond_add,1);

        //tuya_ipc_preset_add_pic(pic_buffer,pic_size); /*if you need show pic ,you should set this api*/
    }
    else if(type->valueint == 2)
    {
        cJSON* num = cJSON_GetObjectItem(data, "num"); //can delete one or more
        cJSON* sets = cJSON_GetObjectItem(data, "sets");
        char respond_del[128] = {0}; 
        cJSON* del_data;
        int del_num = num->valueint;
        for(i = 0; i < del_num; i++)
        {
            del_data = cJSON_GetArrayItem(sets,i);
            cJSON* devId = cJSON_GetObjectItem(del_data, "devId");  /*devid is the preset point number registered in the server*/
            cJSON* mpId = cJSON_GetObjectItem(del_data, "mpId");  /*mpid is the preset point number managed on the device*/
            if((NULL == devId) || (NULL == mpId))
            {
                PR_INFO("devid or mpid is error\n");
                return -1;
            }
            del_preset.seq = atoi(mpId->valuestring);
            
            PR_INFO("%d---%s\n",del_preset.seq,devId->valuestring);

            error_num = (int)time(NULL);

            tuya_ipc_preset_del(devId->valuestring);
            
            snprintf(respond_add,128,"{\"type\":%d,\"data\":{\"error\":%d}}",type->valueint,error_num);
        }
    }
    else if(type->valueint == 3)
    {
        cJSON* mpId = cJSON_GetObjectItem(data, "mpId");

        preset_seq = atoi(mpId->valuestring);
        //get your seq pos and go there
    }
#endif
    return;
}

#endif

#ifdef TUYA_DP_PATROL_STATE
void IPC_APP_patrol_state(int *patrol_state)
{
    //PR_INFO("patrol_state %d\r\n",atoi(patrol_state));
    //return your patrol_state
    return;
}

#endif


#ifdef TUYA_DP_LINK_MOVE_SET
VOID IPC_APP_set_link_pos(INT_T bind_seq)
{
    /*set the link pos*/
    PR_INFO("IPC_APP_set_bind_pos:%d \r\n", bind_seq);
    /*demo
    step1: get the current position
    step2: save the position to flash
    */
    return;
}
#endif


#ifdef TUYA_DP_LINK_MOVE_ACTION
VOID IPC_APP_set_link_move(INT_T bind_seq)
{
    /*move to the link pos*/
    PR_INFO("IPC_APP_set_bind_move:%d \r\n", bind_seq);
    /*demo
     step1: get the position base seq
     step2: go to the target position
    */
    return;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_BLUB_SWITCH
VOID IPC_APP_set_blub_onoff(BOOL_T blub_on_off)
{
    PR_INFO("set blub_on_off:%d \r\n", blub_on_off);
    //TODO
    /* light control switch, BOOL type,true means on,false menas off */

    __tuya_app_write_INT("tuya_blub_on_off", blub_on_off);
}

BOOL_T IPC_APP_get_blub_onoff(VOID)
{
    BOOL_T blub_on_off = __tuya_app_read_INT("tuya_blub_on_off");
    PR_INFO("curr blub_on_off:%d \r\n", blub_on_off);
    return blub_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ELECTRICITY
INT_T IPC_APP_get_battery_percent(VOID)
{
    //TODO
    /* battery power percentage VALUE type,[0-100] */

    return 80;
}
#endif

#ifdef TUYA_DP_POWERMODE
CHAR_T *IPC_APP_get_power_mode(VOID)
{
    //TODO
    /* Power supply mode, ENUM type, 
    "0" is the battery power supply state, "1" is the plug-in power supply state (or battery charging state) */

    return "1";
}
#endif

#if defined(WIFI_GW) && (WIFI_GW==1)
#ifdef TUYA_DP_AP_MODE
STATIC CHAR_T response[128] = {0};
#define WIFI_SSID_LEN 32    // tuya sdk definition WIFI SSID MAX LEN
#define WIFI_PASSWD_LEN 64  // tuya sdk definition WIFI PASSWD MAX LEN
CHAR_T * IPC_APP_get_ap_mode(VOID)
{
    // CHAR_T ap_ssid[WIFI_SSID_LEN + 1] = {0};
    // CHAR_T ap_pw[WIFI_PASSWD_LEN + 1] = {0};
    // WF_WK_MD_E cur_mode = WWM_STATION;
    // NW_MAC_S mac = {{0}};
    // tkl_wifi_get_work_mode(&cur_mode);
    // INT_T is_ap = 0;
    // if(cur_mode == WWM_SOFTAP)
    // {
    //     is_ap = 1;
    // }
    // OPERATE_RET op_ret = tkl_wifi_get_mac(WF_AP,&mac);
    // if(OPRT_OK != op_ret) {
    //     strncpy(ap_ssid, "TUYA_IPC_AP", WIFI_SSID_LEN);
    // }
    // else
    // {
    //     snprintf(ap_ssid, WIFI_SSID_LEN, "TUYA_IPC-%02X%02X",mac.mac[4],mac.mac[5]);
    // }
    // __tuya_app_read_STR("tuya_ap_passwd", ap_pw, WIFI_PASSWD_LEN);

    // snprintf(response, 128, "{\"is_ap\":%d,\"ap_ssid\":\"%s\",\"password\":\"%s\"}",is_ap,ap_ssid,ap_pw);
    // return response;
    return NULL;
}
#endif
#ifdef TUYA_DP_AP_SWITCH
VOID* __ap_change_thread(void *arg)
{
    // WF_WK_MD_E cur_mode = WWM_STATION;
    // tkl_wifi_get_work_mode(&cur_mode);
    // WF_AP_CFG_IF_S ap_cfg;
    // INT_T ap_onoff = 0;
    // tal_system_sleep(5000);

    // ap_onoff = __tuya_app_read_INT("tuya_ap_on_off");
    // memset(&ap_cfg, 0, SIZEOF(WF_AP_CFG_IF_S));
    // PR_INFO("%s : ap_onoff:%d\n", __func__, ap_onoff);
    // if(ap_onoff)
    // {
    //     //ap, sta, monitor, do same
    //     __tuya_app_read_STR("tuya_ap_ssid", (CHAR_T*)ap_cfg.ssid, WIFI_SSID_LEN);
    //     __tuya_app_read_STR("tuya_ap_passwd", (CHAR_T*)ap_cfg.passwd, WIFI_PASSWD_LEN);

    //     ap_cfg.s_len = strlen((CHAR_T *)(ap_cfg.ssid));
    //     ap_cfg.ssid_hidden = 0;
    //     ap_cfg.p_len = strlen((CHAR_T *)(ap_cfg.passwd));
    //     if(ap_cfg.p_len == 0)
    //     {
    //         ap_cfg.md = WAAM_OPEN;
    //     }
    //     else
    //     {
    //         ap_cfg.md = WAAM_WPA2_PSK;
    //     }

    //     tkl_wifi_start_ap(&ap_cfg);
    // }
    // else
    // {
    //     if(cur_mode == WWM_SOFTAP)
    //     {
    //         tkl_wifi_stop_ap();
    //     }
    //     else
    //     {
    //         //not ap mode,do nothing
    //     }
    // }

    return NULL;
}

VOID change_ap_process()
{
    // pthread_t ap_change_thread;
    // int ret = pthread_create(&ap_change_thread, NULL, __ap_change_thread, NULL);
    // if(ret != 0)
    // {
    //    PR_ERR("ap_change_thread ,create fail! ret:%d\n",ret);
    //    return;
    // }

    // pthread_detach(ap_change_thread);
}

INT_T IPC_APP_set_ap_mode(IN cJSON *p_ap_info)
{
    if (NULL == p_ap_info){
        return 0;
    }

    INT_T ap_onoff = -1;

    PR_INFO("%s %d handle_DP_AP_SWITCH:%s \r\n",__FUNCTION__,__LINE__, (char *)p_ap_info);

    cJSON * pJson = cJSON_Parse((CHAR_T *)p_ap_info);

    if (NULL == pJson){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        return ap_onoff;
    }
    cJSON * tmp = cJSON_GetObjectItem(pJson, "ap_enable");
    if (NULL == tmp){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        cJSON_Delete(pJson);
        return ap_onoff;
    }
    ap_onoff = tmp->valueint;
    __tuya_app_write_INT("tuya_ap_on_off", ap_onoff);

    if(ap_onoff == 1)
    {
        cJSON *tmpSsid = cJSON_GetObjectItem(pJson, "ap_ssid");
        if (NULL == tmpSsid){
            PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            return ap_onoff;
        }
        PR_INFO("###[%d] get ssid:%s\n", ap_onoff, tmpSsid->valuestring);

        cJSON *tmpPwd = cJSON_GetObjectItem(pJson, "ap_pwd");
        if (NULL == tmpPwd){
            PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            return ap_onoff;
        }
        PR_INFO("###get pwd:%s\n", tmpPwd->valuestring);

        __tuya_app_write_STR("tuya_ap_ssid", tmpSsid->valuestring);
        __tuya_app_write_STR("tuya_ap_passwd", tmpPwd->valuestring);
    }

    cJSON_Delete(pJson);

    return ap_onoff;
}

CHAR_T * IPC_APP_update_ap_mode(INT_T is_ap)
{
    // CHAR_T ap_ssid[WIFI_SSID_LEN + 1] = {0};
    // CHAR_T ap_pw[WIFI_PASSWD_LEN + 1] = {0};
    // NW_MAC_S mac = {{0}};
    // OPERATE_RET op_ret = tkl_wifi_get_mac(WF_AP,&mac);
    // if(OPRT_OK != op_ret) {
    //     strncpy(ap_ssid, "TUYA_IPC_AP", WIFI_SSID_LEN);
    // }
    // else
    // {
    //     snprintf(ap_ssid, WIFI_SSID_LEN, "TUYA_IPC-%02X%02X",mac.mac[4],mac.mac[5]);
    // }
    // __tuya_app_read_STR("tuya_ap_passwd", ap_pw, WIFI_PASSWD_LEN);

    // snprintf(response, 128, "{\"is_ap\":%d,\"ap_ssid\":\"%s\",\"password\":\"%s\"}",is_ap,ap_ssid,ap_pw);
    // return response;
    return NULL;
}

INT_T IPC_APP_get_ap_mode_config()
{
    return __tuya_app_read_INT("tuya_ap_on_off");
}

INT_T IPC_APP_switch_ap_mode()
{
    // WF_AP_CFG_IF_S ap_cfg = {0};
    // //ap, sta, monitor, do same
    // __tuya_app_read_STR("tuya_ap_ssid", (CHAR_T*)ap_cfg.ssid, WIFI_SSID_LEN);
    // __tuya_app_read_STR("tuya_ap_passwd", (CHAR_T*)ap_cfg.passwd, WIFI_PASSWD_LEN);

    // ap_cfg.s_len = strlen((CHAR_T *)(ap_cfg.ssid));
    // ap_cfg.ssid_hidden = 0;
    // ap_cfg.p_len = strlen((CHAR_T *)(ap_cfg.passwd));
    // if(ap_cfg.p_len == 0)
    // {
    //     ap_cfg.md = WAAM_OPEN;
    // }
    // else
    // {
    //     ap_cfg.md = WAAM_WPA2_PSK;
    // }

    // return tkl_wifi_start_ap(&ap_cfg);

    return 0;
}
#endif

#ifdef TUYA_DP_AP_TIME_SYNC
STATIC BOOL_T time_sync = FALSE;
VOID IPC_APP_set_time(IN CHAR_T *p_time_info)
{
    if (NULL == p_time_info){
        return;
    }
    if(time_sync == TRUE)
    {
        return;
    }        
    PR_INFO("%s %d recv time sync from app:%s \r\n",__FUNCTION__,__LINE__, p_time_info);

    tuya_ipc_set_service_time(atoi(p_time_info));
    time_sync = TRUE;
    return;
}
#endif

#ifdef TUYA_DP_AP_TIME_ZONE
STATIC BOOL_T timezone_sync = FALSE;
VOID IPC_APP_set_time_zone(IN CHAR_T *p_time_info)
{
    if (NULL == p_time_info){
        return;
    }
    PR_INFO("%s %d recv timeZONE sync from app:%s \r\n",__FUNCTION__,__LINE__, p_time_info);
    if(timezone_sync == TRUE)
    {
        return;
    }        

    extern OPERATE_RET tal_time_set_time_zone(IN CONST CHAR_T *time_zone);
    tal_time_set_time_zone(p_time_info);
    
    //TODO: You need call your own timezone setter function here.

    timezone_sync = TRUE;
    return;
}
#endif
#endif

