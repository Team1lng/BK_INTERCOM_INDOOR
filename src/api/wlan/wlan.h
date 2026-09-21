//
// Created by michael on 2021/11/8.
//

#ifndef KCV_T701BT_WLAN_H
#define KCV_T701BT_WLAN_H
#include <stdbool.h>

#define MAX_WIFI_SCAN  32

typedef struct {
    char ssid[128];
    char flags[128];
    int signal_level;
    bool free;
    bool hidden;
}wifi_info;

typedef struct {

    int level;
    char completed;
    char key_mgmt[64];
    char wlan_ssid[64];
    char ip[32];
    char mac[32];
}linked_info;

void wifi_usb_module_init(void);

bool wifi_usb_module_enable(void);

bool create_check_wlan_task(void);//创建检查wifi的线程

bool destroy_check_wlan_task(void);//销毁检查wifi的线程

int wpa_cli_check_connect_status(void);//检查连接的状态-1: psk error 1: connect  0:other error

void wpa_cli_connect_new_wifi(char* ssid,char *psk);//连接新的wifi 成功返回true 失败返回false

int wpa_cli_scan_wifi(bool *continue_flag);//搜索wifi 并且返回wifi数量 失败-1

bool get_scanned_wifi_info(int index ,wifi_info* info);//获取指定结点的搜索的wifi结点

void get_linked_wifi_info(linked_info* info);//获取连接的wifi的信息

void turn_on_wlan_connect(void);//打开wifi连接

void turn_off_wlan_connect(void);//关闭wifi连接

void update_wifi_list(void);//更新wifi列表

int get_max_wifi_list_index(void);//获取wifi列表内的wifi数量

void turn_on_walan_reconnect(void);

void turn_off_wlan_break(void);

int wifi_connection_status_sucess(void);

bool wpa_cli_wlan_status(bool *continue_flag);

void CloseConnectingWifi(void);


void StartConnectingWifi(void);

#endif //KCV_T701BT_WLAN_H
