#ifndef _NETWORK_COMMON_H_
#define _NETWORK_COMMON_H_
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#define NETWORK_NAME "br0"
#define ETH0_MAC 0xFD
#define ETH1_MAC 0xFE

struct net_block_desc
{
	uint32_t version;
	uint32_t offset_to_priv;
	struct tpacket_hdr_v1 h1;
};

struct net_ring_buffer
{
	struct iovec *rd;
	uint8_t *map;
	struct tpacket_req3 req;
};

typedef enum
{
	/* 室内机与门口机通话 */
	NETWORK_EVENT_OUTDOOR_TALK,

} network_event;

typedef struct
{
	bool talk_busy;
	int ver;
	int Compile_year;
	int Compile_mon;
	int Compile_day;

	bool fingerprint_module;
} door_staus_info;

#define network_get_id_slave_id(x, y) ((x & 0x00) | ((y)-1))
#define network_get_id_indoor_id1(x) ((x & 0x00) | (0x0))
#define network_get_id_indoor_id2(x) ((x & 0x00) | (0x1))
#define network_get_id_indoor_id3(x) ((x & 0x00) | (0x2))
#define network_get_id_indoor_id4(x) ((x & 0x00) | (0x3))
#define network_get_id_indoor_id5(x) ((x & 0x00) | (0x4))
#define network_get_id_indoor_id6(x) ((x & 0x00) | (0x5))

#define network_get_id_outdoor1(x) ((x & 0x00) | (0x6))
#define network_get_id_outdoor2(x) ((x & 0x00) | (0x7))

#define network_get_id_cctv1(x) ((x & 0x00) | (0x8))
#define network_get_id_cctv2(x) ((x & 0x00) | (0x9))

typedef enum
{
	DEVICE_UNKONW,
	DEVICE_INDOOR_ID1 = 1,
	DEVICE_INDOOR_ID2,
	DEVICE_INDOOR_ID3,
	DEVICE_INDOOR_ID4,
	DEVICE_INDOOR_ID5,
	DEVICE_INDOOR_ID6,
	DEVICE_OUTDOOR_1,
	DEVICE_OUTDOOR_2,
	DEVICE_CCTV_1,
	DEVICE_CCTV_2,
	DEVICE_END,
	DEVICE_ALL = 0XFF,
	DEVICE_TOTAL
} network_device;

typedef enum
{
	FAMILY_UNKONW,
	FAMILY_ID1,
	FAMILY_ID2,
	FAMILY_ID3,
	FAMILY_ID4,
	FAMILY_TOTAL
} network_family;

#define COMMON_CMD_LEN 8

/*
 *	arg1: 1:查询ID状态
 *
 *		  2:收到ID状态
 *
 *			   arg2:最高位1表示id，冲突，低4位表示冲突的ID号
 *					最高位0:表示该id存在，但不冲突
 */
#define NET_COMMON_CMD_LIGHT 0x53

#define NET_COMMON_CMD_UNLOCK 0x54

#define NET_COMMON_CMD_ID_REPEAT 0x55

#define NET_COMMON_CMD_OUTDOOR_CALL 0X56

/*
 *
 * arg1:当前想通话的通道
 *
 */
#define NET_COMMON_CMD_OUTDOOR_TALK 0X57

#define NET_COMMON_CMD_OUTDOOR_HANG 0X75
/*
 *	arg1 = 1:呼叫
 *   arg1 = 2:有设备应答呼叫
 *	arg1 = 3:有设备接受通话
 *   arg1 = 4:设备挂断正在通话
 *	arg1 = 5:设备正忙
 */
#define NET_COMMON_CMD_INTERCOM_CALL 0X58
#define NET_COMMON_CMD_STREAM_STATUS 0X59
/*
 * arg1= 1:代表设备正在与门口机通话 arg2 = 当前通话的通道
 */
#define NET_COMMON_CMD_DEVICE_BUSY 0X60
#define NET_COMMON_CMD_MOTION 0X61
#define NET_COMON_CMD_UPGRADE_OUTDOOR 0x62

#define NET_COMON_CMD_MOTION_SENSITIVITY 0x63

#define NET_COMON_CMD_ADD_DEL_CARD 0x64

#define NET_COMON_CMD_OUTDOOR_RESET 0x65

#define NET_COMMON_CMD_MAILBOX_STATUS 0X66

/* 播放留言提示音 */
#define NET_COMMON_PARAM_RECORD_MESSAGE (1 << 1)
/* 視頻通話 */
#define NET_COMMON_PARAM_CAMERA_TALK (1 << 2)
/* 涂鸦监控 */
#define NET_COMMON_PARAM_CAMERA_TUYA (1 << 3)
/* 開鎖提示 */
#define NET_COMMON_PARAM_UNLOCK_HINT (1 << 4)

#define NET_COMMON_CMD_NONE 0X00

#define NET_COMMON_CMD_COMPILE_TIME 0X70

#define NET_COMMON_CMD_DEF_UNLOCK_TIME 0X71

#define NET_COMMON_CMD_EXIT_BUTTON_TIME 0X72

#define NET_COMON_CMD_ADD_FINEGR_ACTION 0x80
#define NET_COMON_CMD_DEL_FINEGR_ACTION 0x81
#define NET_COMON_CMD_CANCEL_FINEGR_ACTION 0x82

#define NET_COMMON_CMD_GATE2_UNLOCK 0X99
typedef struct
{
	network_device device;
	char cmd;
	char arg1;
	char arg2;
} network_cmd_data;

#define UPGRADE_PACK_LEN 512

typedef struct
{
	network_device device;
	char cmd;
	int arg1;
	int arg2;
	char buf[UPGRADE_PACK_LEN * 2];
} network_upgradecmd_data;

#define netwrok_cmd_create(a, b, c, d) { \
	.device = a,                         \
	.cmd = b,                            \
	.arg1 = c,                           \
	.arg2 = d};

bool network_init(network_device device);

void outdoor_order_set(int cmd);
void network_local_device_set(network_device device);
void network_local_family_set(int family);
void network_local_mac_set(void);
int setMacAddress(const char *interfaceName, const char *newMacAddress);

network_device network_local_device_get(void);

int get_outdoor_version(network_device ch);

bool get_outdoor_finerger_status(network_device ch);

door_staus_info *get_outdoor_info(int ch);

bool network_send_cmd_data(network_cmd_data *package);

bool device_enable_state_get(network_device device);
void device_enable_state_set(network_device device, bool *enable);
void device_cctv_url_set(network_device device, char *url);
bool device_online_state_get(network_device device);
bool family_dev_online_state_get(network_family family, network_device device);
bool device_repeat_state_get(void);
bool tuya_monitor_state_get(void);

bool network_video_send_package_open(unsigned long id);
bool network_video_send_package_push(char type, const char *data, int len);
bool network_video_send_package_close(void);

bool network_video_receive_package_open(unsigned long id);
bool network_video_receive_package_close(void);
int curr_network_video_receive_eth_id_get(void);

bool network_audio_send_package_open(unsigned long id);
bool network_audio_send_package_close(void);
bool network_audio_send_package_push(char type, const char *data, int len, bool ring_back);

bool network_audio_receive_package_open(unsigned long);
bool network_audio_receive_package_close(void);

int network_common_socket_eth_p_get(char type, int slave_id, int family);

bool network_sendupgrade_cmd_data(network_upgradecmd_data *data);

int network_audio_send_task_eth_id(void);
int network_audio_receive_task_eth_id(void);

void request_send_I_frame_cmd(network_device ch);
#endif
