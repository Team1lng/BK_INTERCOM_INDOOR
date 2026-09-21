#ifndef _TCP_NETWORK_CMD_H_
#define _TCP_NETWORK_CMD_H_

#include"stdbool.h"
#include"stdio.h"
#include"ak_thread.h"


#define FINGER_REMARK_PATH  "/tmp/finger_readme.conf"
#define CARD_REMARK_PATH  "/tmp/card_readme.conf"

#define SHORT_PACK_LEN 8
#define LONG_PACK_LEN   1288

#define DATA_PACK_START 0xAA
#define LONG_PACK_START 0xBB
#define FILE_PACK_START 0xAB
#define PACK_END 0xCC

#define FAIL_RET 9
#define CONTINUE_RET 0
#define SUCCEE_RET 1
#define SATUTS_SWICTH 2


#define NET_COMMON_CMD_ACK        0x01
#define NET_COMMON_CMD_ADD_FINGER        0x02
#define NET_COMMON_CMD_EXIT_FINGER        0x07
#define NET_COMMON_CMD_DEL_FINGER        0x03
#define NET_COMMON_CMD_VERIFY_FINGER        0x04
#define NET_COMMON_CMD_GET_FINGER        0x05
#define NET_COMMON_CMD_SET_FINGER_PERMISSION        0x06
#define NET_COMMON_CMD_FINGER_REMARK_GET        0x08
#define NET_COMMON_CMD_FINGER_REMARK_SEND        0x09


#define NET_COMMON_CMD_ADD_CARD        0x10
#define NET_COMMON_CMD_EXIT_CARD        0x15
#define NET_COMMON_CMD_DEL_CARD        0x11
#define NET_COMMON_CMD_VERIFY_CARD        0x12
#define NET_COMMON_CMD_GET_CARD        0x13
#define NET_COMMON_CMD_SET_CARD_PERMISSION	0x14
#define NET_COMMON_CMD_CARD_REMARK_PULL        0x16
#define NET_COMMON_CMD_CARD_REMARK_RESPONSE        0x17

#define NET_COMMON_CMD_ADD_PASSW        0x20
#define NET_COMMON_CMD_DEL_PASSW        0x21
#define NET_COMMON_CMD_GET_PASSW        0x22
#define NET_COMMON_CMD_SET_PASSW_PERMISSION        0x23

#define NET_COMMON_CMD_ACCESS_DENIED	0x90

#define LOCAL_DEVICE    1


#define OUTDOOR1_ID     0
#define OUTDOOR2_ID     1

typedef enum
{
	MANAGE_CARD,
	MANAGE_FINGER,
	MANAGE_PASSW,
}management_type;

typedef enum
{
	// TCP_DEVICE_INDOOR_1 = 1,
	// TCP_DEVICE_INDOOR_2,
	// TCP_DEVICE_INDOOR_3,
	// TCP_DEVICE_INDOOR_4,
	// TCP_DEVICE_INDOOR_5,
	// TCP_DEVICE_INDOOR_6,
	TCP_DEVICE_OUTDOOR_1,
	TCP_DEVICE_OUTDOOR_2,
	TCP_DEVICE_END,
	TCP_DEVICE_ALL = 0XFF,
	TCP_DEVICE_TOTAL
}tcp_device;

typedef struct 
{
    unsigned char cmd;
    unsigned char arg1;
    unsigned char arg2;

    char *data;
    unsigned int data_size;

	tcp_device dev_id;
	ak_pthread_t thread_id ;
	int dev_fd;
}tcp_network_info;


typedef struct
{
	char* str;
	unsigned char cmd;
	void(*proc)(const tcp_network_info *dev);
}net_common_event_info;

// bool network_short_send_common(tcp_device send_to_devce,unsigned char cmd,unsigned char arg1,unsigned char arg2);

// bool network_long_package_data(tcp_device send_to_device, unsigned char cmd,unsigned char *code,unsigned char arg);


void tcp_network_info_init(void);

bool tcp_management_init(tcp_device device_id);

bool tcp_management_close(void);

bool tcp_network_cmd_get_data_info_send(tcp_device send_to_device);
bool tcp_network_cmd_add_data_send(tcp_device send_to_device,unsigned char arg1,unsigned char arg2);
bool tcp_network_cmd_exit_send(tcp_device send_to_device);
bool tcp_network_cmd_del_data_send(int index,tcp_device send_to_device);
bool tcp_network_cmd_set_lock_type_send(int index, uint8_t lock_type,tcp_device send_to_device);
management_type data_manage_type_get(void);
void data_manage_type_set(management_type type);
char *data_packge_buffer_get(void);
bool management_create_state(tcp_device dev);
#endif