/*********************************************************************************
 *Copyright(C),2015-2020,
 *TUYA
 *FileName: tuya_ipc_dp_.c
 *
 * File Description：
 * 1. API implementation of DP point
 *
 * This file code is the basic code, users don't care it
 * Please do not modify any contents of this file at will.
 * Please contact the Product Manager if you need to modify it.
 *
 **********************************************************************************/
#include "utilities/uni_log.h"
#include "tuya_iot_config.h"
#include "tuya_ipc_api.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_iot_config.h"
#include "ty_dp_define.h"
#include "ty_sdk_common.h"
#include "utilities/uni_log.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define TUYA_UNPACK_ENABLE 0
#if defined(TUYA_UNPACK_ENABLE) && (TUYA_UNPACK_ENABLE == 1)
#include "tuya_unpack.h"

ota_stream *ota_obj = NULL;
#endif

/* OTA */
// Callback after downloading OTA files
OPERATE_RET __IPC_APP_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    FILE *p_upgrade_fd = (FILE *)pri_data;
    if (p_upgrade_fd)
    {
        fclose(p_upgrade_fd);
    }

    printf("Upgrade Finish\n\r");
    printf("download_result:%d fw_url:%s\n\r", download_result, fw->fw_url);

    if (download_result == 0)
    {
        /* The developer needs to implement the operation of OTA upgrade,
        when the OTA file has been downloaded successfully to the specified path. [ p_mgr_info->upgrade_file_path ]*/
    }
    if (fw->tp == DEV_NM_ATH_SNGL) // 室内机升级包，开始升级室内机
    {
        extern int ak_drv_wdt_close(void);
        ak_drv_wdt_close();
        system("cp /etc/config/wpa_supplicant.conf /app/data");

        // 从 fw_url 解析版本标记名 SAT_ANYKAOS_xxxxxxxx 传给 update.sh，
        // 使升级成功后 update.sh 能把版本号 touch 到 /app/data。
        // update.sh 内写标记的动作被 `-n "$1"` 守卫，之前不带参数调用导致
        // OTA 后 /app/data 版本文件不更新，进而 SD 卡升级判断错乱。
        char ver_name[64] = {0};
        char upgrade_cmd[256] = {0};
        const char *ver_pos = strstr(fw->fw_url, "SAT_ANYKAOS_");
        int idx = 0;

        if (ver_pos != NULL)
        {
            for (; ver_pos[idx] != '\0' && idx < (int)sizeof(ver_name) - 1; idx++)
            {
                if (ver_pos[idx] == '_' ||
                    (ver_pos[idx] >= '0' && ver_pos[idx] <= '9') ||
                    (ver_pos[idx] >= 'A' && ver_pos[idx] <= 'Z'))
                {
                    ver_name[idx] = ver_pos[idx];
                }
                else
                {
                    break;
                }
            }
        }
        ver_name[idx] = '\0';
        if (idx == 0)
        {
            snprintf(ver_name, sizeof(ver_name), "SAT_ANYKAOS_UNKNOWN");
        }
        printf("OTA upgrade ver_name:%s\n\r", ver_name);

        snprintf(upgrade_cmd, sizeof(upgrade_cmd),
                 "tar -zxvf /tmp/TWO_WIRE_APP -C /tmp;killall daemon.sh;killall app.sh;killall ANYKA37E.BIN;/tmp/update.sh %s &",
                 ver_name);
        system(upgrade_cmd);
    }
    else // 门口机升级包，开始升级门口机
    {
        system("mv /tmp/TWO_WIRE_APP /tmp/cbin.update");
        extern bool tuya_ota_upgrade_event(void);
        tuya_ota_upgrade_event();
    }

    // TODO
    // reboot system
    return OPRT_OK;
}

// To collect OTA files in fragments and write them to local files
OPERATE_RET __IPC_APP_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                       IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    // printf("Rev File Data:\n\r");
    // printf("total_len:%d  fw_url:%s\n\r", total_len, fw->fw_url);
    // printf("Offset:%d Len:%d\n\r", offset, len);

    // report UPGRADE process, NOT only download percent, consider flash-write time
    // APP will report overtime fail, if uprgade process is not updated within 60 seconds

    int download_percent = (offset * 100) / (total_len + 1);
    int report_percent = download_percent; // as an example, download 100% = 50%  upgrade work finished

    tuya_ipc_upgrade_progress_report_by_type(report_percent, fw->tp);
    // tuya_ipc_upgrade_progress_report(report_percent);

    FILE *p_upgrade_fd = (FILE *)pri_data;
    if (p_upgrade_fd)
    {
        fwrite(data, 1, len, p_upgrade_fd);
    }
    // APP will report "uprage success" after reboot and new FW version is reported inside SDK automaticlly

    return OPRT_OK;
}

INT_T TUYA_IPC_Upgrade_Inform_cb(IN CONST FW_UG_S *fw)
{
    printf("Rev Upgrade Info \n\r");
    printf("fw->fw_url:%s \n\r", fw->fw_url);
    printf("fw->fw_md5:%s \n\r", fw->fw_md5);
    printf("fw->sw_ver:%s \n\r", fw->sw_ver);
    printf("fw->file_size:%u \n\r", fw->file_size);
    printf("fw->fw_hmac:%s \n\r", fw->fw_hmac);
    FILE *p_upgrade_fd = fopen("/tmp/TWO_WIRE_APP", "w+b");
    tuya_ipc_upgrade_sdk(fw, __IPC_APP_get_file_data_cb, __IPC_APP_upgrade_notify_cb, p_upgrade_fd);

    return OPRT_OK;
}