/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-04-18 17:33:40
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-04-27 08:26:11
 * @FilePath: /two-wire-indoor/src/api/tcp_network/tcp_upgrade.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef TCP_UPGRADE_H
#define TCP_UPGRADE_H
#include <stdbool.h>

typedef enum
{
    NoneUpgrade,
    StartUploading,
    VersionConsistent,
    UploadFail,
    StartUpgrade,
    UpgradeFinish,
    UpgradeFail,
} upgrade_status;

bool CreateUpgradeClientTask(char *server_ip, char *path);

upgrade_status GetUpgradeProgress(void);
#endif