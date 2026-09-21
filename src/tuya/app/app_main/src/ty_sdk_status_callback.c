#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <signal.h>
#include <sys/prctl.h>

#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "ty_sdk_common.h"
#include "tuya_online_status_policy.h"

STATIC INT_T s_mqtt_status = 0;
STATIC INT_T s_first_active = 0;
static char qrcode_shorturl[128] = {0};
typedef VOID (*ON_IPC_STATUS_CHANGE)(VOID *arg);
typedef struct {
    TUYA_IPC_STATUS_E stat;
    ON_IPC_STATUS_CHANGE cb;
}IPC_STATUS_CHANGE_CB_MAP_T;


STATIC VOID __on_register_fail(VOID *arg)
{
    
    db_log_warn("%s, get registe fail status.", __func__);

    return;
}

STATIC VOID __on_netcfg_start(VOID *arg){

    db_log_debug("%s, netcfg start. You can start your own netcfg task now.", __func__);

    return;
}

STATIC VOID __on_netcfg_stop(VOID *arg){
    
    db_log_debug("%s, netcfg stop. You can stop your own netcfg task now.", __func__);
    s_first_active = 1;
    return;
}

STATIC VOID __on_status_offline(VOID *arg){

    db_log_warn("offline: network status MQTT disconnected"); //according to requirements of shadow device, do not modify!!!
    if(s_mqtt_status)
    {
        s_mqtt_status = 0;
    }
    extern bool dev_info_status_event_push(unsigned long arg1, unsigned long arg2);
    dev_info_status_event_push(1, 0);
    return;
}

STATIC VOID __on_status_fw_upgrade_start(VOID *arg){

    db_log_debug("fw upgrade: recv start cmd"); 
    return;
}

STATIC VOID __on_status_online(VOID *arg){

    db_log_debug("online: network status MQTT connected"); //according to requirements of shadow device, do not modify!!!
    s_mqtt_status = 1 + s_first_active;
    extern bool dev_info_status_event_push(unsigned long arg1, unsigned long arg2);
    dev_info_status_event_push(1, 0);
    s_first_active = 0;
    return;
}


static IPC_STATUS_CHANGE_CB_MAP_T status_change_cb_map[] = {
    {TUYA_IPC_STATUS_REGISTER,          NULL},
    {TUYA_IPC_STATUS_REGISTER_FAILED,   __on_register_fail},
    {TUYA_IPC_STATUS_ACTIVED,           NULL},
    {TUYA_IPC_STATUS_RESET,             NULL},
    {TUYA_IPC_STATUS_NETCFG_START,      __on_netcfg_start},
    {TUYA_IPC_STATUS_NETCFG_STOP,       __on_netcfg_stop},
    {TUYA_IPC_STATUS_WIFI_STA_UNCONN,   NULL},
    {TUYA_IPC_STATUS_WIFI_STA_CONN,     NULL},
    {TUYA_IPC_STATUS_WIRE_UNCONN,       NULL},
    {TUYA_IPC_STATUS_WIRE_CONN,         NULL}, 
    {TUYA_IPC_STATUS_ONLINE,            __on_status_online},
    {TUYA_IPC_STATUS_OFFLINE,           __on_status_offline},    
    {TUYA_IPC_STATUS_UPGRADE_START,     __on_status_fw_upgrade_start}, 
};

VOID TUYA_IPC_Status_Changed_cb(IN TUYA_IPC_STATUS_GROUP_E changed_group, IN CONST TUYA_IPC_STATUS_E status[TUYA_IPC_STATUS_GROUP_MAX])
{
    TUYA_IPC_STATUS_E cur_status = status[changed_group];
    INT_T i;
    PR_DEBUG("status chaged: group[%d] status[%d]", changed_group, cur_status);
    
    for(i = 0; i < SIZEOF(status_change_cb_map)/SIZEOF(IPC_STATUS_CHANGE_CB_MAP_T); i++) {
        if(status_change_cb_map[i].stat == cur_status) {
            if(status_change_cb_map[i].cb) {
                status_change_cb_map[i].cb(NULL);
            } else {
                PR_DEBUG("status ignore. cb is null.");
            }
        }
    }    
}

INT_T TUYA_IPC_sd_status_upload(INT_T status)
{
	IPC_APP_report_sd_status_changed(status);
	return 0;
}

VOID TUYA_IPC_qrcode_shorturl_cb(CHAR_T* shorturl)
{
    if(shorturl)
    {
        char *p_str = strstr(shorturl, "https");
        if (p_str)
        {
            strncpy(qrcode_shorturl, p_str, strlen(p_str) - 2);
            db_log_debug("qrcode_shorturl: %s\n", qrcode_shorturl); // according to requirements of shadow device, do not modify!!!
        }
    }
    return;
}


/* 
Callback when the user clicks on the APP to remove the device
*/
VOID TUYA_IPC_Reset_System_CB(GW_RESET_TYPE_E type)
{
    db_log_debug("reset ipc success. please restart the ipc %d\n", type);
    system("rm -rf " TUYA_CACHE_PATH "*");
    backlight_open(false, false, 0);
    extern int lcd_reset_pin_higt(void);
    lcd_reset_pin_higt(); /* 防止上电复位失败 */
    ak_sleep_ms(100);
    system("reboot");
    //TODO
    /* Developers need to restart IPC operations */
}

VOID TUYA_IPC_Restart_Process_CB(VOID)
{
    db_log_debug("sdk internal restart request. please restart the ipc\n");
    backlight_open(false, false, 0);
    system("reboot");
    //TODO
    /* Developers need to implement restart operations. Restart the process or restart the device. */
}

INT_T TUYA_IPC_Get_MqttStatus()
{
    return s_mqtt_status;
}

const char *tuya_qrcode_shorturl_get(void)
{
    if (qrcode_shorturl[0])
    {
        return qrcode_shorturl;
    }
    return NULL;
}

bool tuya_online_status_get(void)
{
    return tuya_online_status_from_callback(s_mqtt_status);
}
