#include "network_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
// #include<netinet/ip.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "leo_api.h"
#include "tuya_sdk.h"
#include "debug_printf.h"

bool dev_info_status_event_push(unsigned long arg1, unsigned long arg2);

#define NET_COMMON_CMD_START 0XAA
#define NET_COMMON_CMD_END 0X55

#define ETH_P_CMD 0xFFFF // 0x0800//
#define BEATTIM 4		 // 心跳包时间间隔

static int cmd_receive_fd = -1;
static int cmd_send_fd = -1;

static ak_mutex_t network_device_mutex;
static ak_mutex_t outdoor_order_mutex;

typedef struct
{
	bool *enable;
	unsigned long heart_time;
	union
	{
		char *url;
		bool device_onlin_state;
	} info;
} device_online_info;

typedef struct
{
	struct ak_timeval tuya_monitor_time;
	bool tuya_monitor_ing;
} tuya_monitor_info;

static long device_repeat_time = 0; // 接收到冲突ID心跳包时间
static bool local_device_repeat = false;

static tuya_monitor_info tuya_mon_state;

static device_online_info device_heart_info[FAMILY_TOTAL][DEVICE_END];

static network_device network_local_device = DEVICE_UNKONW;
static int device_family_id = 1;

struct
{
	door_staus_info info[2];
} OutDoor_Info;

moniotr_config moniotr_conf = {NULL};

void monitor_device_init(door_info *doo1, door_info *door2, camera_info *cctv1, camera_info *cctv2)
{
	moniotr_conf.outdoor1 = doo1;
	moniotr_conf.outdoor2 = door2;
	moniotr_conf.cctv1 = cctv1;
	moniotr_conf.cctv2 = cctv2;
}

moniotr_config *monitor_config_get(void)
{
	return &moniotr_conf;
}

bool get_outdoor_talk_state(MONITOR_CH ch)
{
	return OutDoor_Info.info[ch == MON_CH_DOOR_1 ? 0 : 1].talk_busy;
}

int get_outdoor_version(network_device ch)
{
	return OutDoor_Info.info[ch == DEVICE_OUTDOOR_1 ? 0 : 1].ver;
}

int get_outdoor_model_version(network_device ch, int *model, int *ver)
{
	int index = ch == DEVICE_OUTDOOR_1 ? 0 : 1;
	if (OutDoor_Info.info[index].model != 0 && OutDoor_Info.info[index].ver != 0)
	{
		if (model)
		{
			*model = OutDoor_Info.info[index].model;
		}
		if (ver)
		{
			*ver = OutDoor_Info.info[index].ver;
		}
		return 0;
	}
	return -1;
}

bool get_outdoor_finerger_status(network_device ch)
{
	return OutDoor_Info.info[ch == DEVICE_OUTDOOR_1 ? 0 : 1].fingerprint_module;
}

door_staus_info *get_outdoor_info(int ch)
{
	if (ch < 2)
		return &(OutDoor_Info.info[ch]);
	else
		return NULL;
}

void set_outdoor_version(network_device ch, int ver)
{
	OutDoor_Info.info[ch == DEVICE_OUTDOOR_1 ? 0 : 1].ver = ver;
}

extern bool device_id_repeat_push(network_device device);

static int outdoor_order_arg2 = 0x00;
void outdoor_order_set(int cmd)
{
	ak_thread_mutex_lock(&outdoor_order_mutex);
	outdoor_order_arg2 = cmd;
	ak_thread_mutex_unlock(&outdoor_order_mutex);
}

void network_local_device_set(network_device device)
{
	network_local_device = device;
	char buffer[128] = {0};
	// sprintf(buffer, "ip addr add dev %s %s/24", NETWORK_NAME, "192.168.37.2");
	sprintf(buffer, "ifconfig  %s 192.168.37.%d%d", NETWORK_NAME, device_family_id, network_local_device);
	printf("%s \n", buffer);
	system(buffer);
}

void network_local_mac_set(void)
{
	char MAC[128] = {0};
	sprintf(MAC, "C2:73:C9:3F:%x:%d%d", ETH0_MAC, device_family_id, network_local_device);
	Debug_Lib("%s\n", MAC);
	setMacAddress("eth0", MAC);
	sprintf(MAC, "C2:73:C9:3F:%x:%d%d", ETH1_MAC, device_family_id, network_local_device);
	setMacAddress("eth1", MAC);
	Debug_Lib("%s\n", MAC);
}

void network_local_family_set(int family)
{
	device_family_id = family;
	char buffer[128] = {0};
	// sprintf(buffer, "ip addr add dev %s %s/24", NETWORK_NAME, "192.168.37.2");
	sprintf(buffer, "ifconfig  %s 192.168.37.%d%d", NETWORK_NAME, device_family_id, network_local_device);
	printf("%s \n", buffer);
	system(buffer);
}

network_device network_local_device_get(void)
{
	return network_local_device;
}
static struct sockaddr_ll nework_local_sockaddr_ll;
struct sockaddr_ll *network_get_send_addres(void)
{
	return &nework_local_sockaddr_ll;
}
static bool network_cmd_socket_init(void)
{ // AF_INET  //PF_PACKET             //SOCK_DGRAM  SOCK_RAW
	if ((cmd_receive_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CMD))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

	struct ifreq req;
	int fd = socket(PF_INET, SOCK_DGRAM, 0); // tcp协议是否可行-(2023.12.01)感觉没影响
	strcpy(req.ifr_name, NETWORK_NAME);
	ioctl(fd, SIOCGIFINDEX, &req);
	close(fd);

	struct sockaddr_ll sll;
	struct packet_mreq mr;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = req.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_CMD);
	if (bind(cmd_receive_fd, (struct sockaddr *)&sll, sizeof(sll)) == -1)
	{
		perror("bind");
		return (1);
	}

	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = req.ifr_ifindex;
	Debug_Lib("%d\n", req.ifr_ifindex);
	mr.mr_type = PACKET_MR_PROMISC; // 配置为混杂模式
	if (setsockopt(cmd_receive_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
	{
		perror("setsockopt");
		return (1);
	}

	int recv_buf_size = 512;
	if (setsockopt(cmd_receive_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(int)) < 0)
	{
		printf("setsockopt SO_RCVBUF\n");
		return false;
	}

	if ((cmd_send_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_CMD))) < 0)
	{
		printf("create socket error raw_socket_send_aduio_fd\n");
		return false;
	}

	memset(&nework_local_sockaddr_ll, 0, sizeof(nework_local_sockaddr_ll));
	/*网卡eth0的index，非常重要，系统把数据往哪张网卡上发，就靠这个标识*/
	nework_local_sockaddr_ll.sll_ifindex = req.ifr_ifindex;

#if 0
	/*标识包的类型为发出去的包*/
	nework_local_sockaddr_ll.sll_pkttype   = PACKET_OUTGOING;
	/*目标MAC地址长度为6*/
	nework_local_sockaddr_ll.sll_halen     = 6;    
	/*填写目标MAC地址*/
	nework_local_sockaddr_ll.sll_addr[0]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[1]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[2]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[3]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[4]   = 0xFF;
	nework_local_sockaddr_ll.sll_addr[5]   = 0xFF;
#endif
	return true;
}

#include <arpa/inet.h>
int setMacAddress(const char *interfaceName, const char *newMacAddress)
{
	int sockfd;
	struct ifreq ifr;

	// 打开套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	// 设置网卡的名称
	strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ - 1);

	// 获取当前的MAC地址
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
	{
		perror("ioctl");
		close(sockfd);
		return -1;
	}

	// 将新的MAC地址字符串转换为二进制形式
	// if (!inet_pton(AF_INET, "6C:60:EB:C4:4C:48", (void*)&ifr.ifr_hwaddr.sa_data)) {
	//     fprintf(stderr, "Invalid MAC address format.\n");
	//     close(sockfd);
	//     return -1;
	// }

	sscanf(newMacAddress, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		   &ifr.ifr_hwaddr.sa_data[0], &ifr.ifr_hwaddr.sa_data[1], &ifr.ifr_hwaddr.sa_data[2],
		   &ifr.ifr_hwaddr.sa_data[3], &ifr.ifr_hwaddr.sa_data[4], &ifr.ifr_hwaddr.sa_data[5]);

	// 设置新的MAC地址
	if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) == -1)
	{
		perror("ioctl");
		close(sockfd);
		return -1;
	}

	close(sockfd);
	return 0;
}

static void network_device_init(network_device device)
{
	// printf("=============>>local device:%d \n", device);
	network_local_device = device;
	if ((network_local_device == DEVICE_INDOOR_ID1) ||
		(network_local_device == DEVICE_INDOOR_ID2) ||
		(network_local_device == DEVICE_INDOOR_ID3) ||
		(network_local_device == DEVICE_INDOOR_ID4) ||
		(network_local_device == DEVICE_INDOOR_ID5) ||
		(network_local_device == DEVICE_INDOOR_ID6))
	{
		system("1 >/proc/sys/net/ipv4/ip_forward");

		system("ip link add name " NETWORK_NAME " type bridge");

		system("ip link set " NETWORK_NAME " up");

		system("ip link set dev eth0 master " NETWORK_NAME);

		system("ip link set dev eth1 master " NETWORK_NAME);

		system("ifconfig eth0 0.0.0.0 promisc");

		system("ifconfig eth1 0.0.0.0 promisc");

		char buffer[128] = {0};
		// sprintf(buffer, "ip addr add dev %s %s/24", NETWORK_NAME, "192.168.37.2");
		sprintf(buffer, "ifconfig  %s 192.168.37.%d%d", NETWORK_NAME, device_family_id, network_local_device);
		printf("%s \n", buffer);
		system(buffer);
	}
	else
	{
		char buffer[128] = {0};
		sprintf(buffer, "ifconfig eth0 %s netmask 255.255.255.0", network_local_device == DEVICE_OUTDOOR_1 ? "192.168.37.7" : "192.168.37.8");
		system(buffer);
	}

	char MAC[128] = {0};
	sprintf(MAC, "C2:73:C9:3F:%x:%d%d", ETH0_MAC, device_family_id, network_local_device);
	Debug_Lib("%s\n", MAC);
	setMacAddress("eth0", MAC);
	sprintf(MAC, "C2:73:C9:3F:%x:%d%d", ETH1_MAC, device_family_id, network_local_device);
	setMacAddress("eth1", MAC);
	Debug_Lib("%s\n", MAC);
	// system("route add -net 224.0.0.0 netmask 224.0.0.0 " NETWORK_NAME);
	network_cmd_socket_init();

	memset(device_heart_info, 0, sizeof(device_heart_info));

	for (network_family family = FAMILY_ID1; family < FAMILY_TOTAL; family++)
	{
		for (network_device dev = DEVICE_INDOOR_ID1; dev < DEVICE_END; dev++)
		{
			device_heart_info[family][dev].enable = NULL;
		}
	}

	device_heart_info[device_family_id][network_local_device].info.device_onlin_state = true;
}

static bool net_common_code_check_valid(const char *buffer)
{
	/*先判断起始码和结束码是否一致*/
	if ((buffer[0] != NET_COMMON_CMD_START) || (buffer[COMMON_CMD_LEN - 1] != NET_COMMON_CMD_END))
	{
		return false;
	}

	/*判断校验和是否正确*/
	unsigned char num = (buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5]) & 0xFF;
	if (num != buffer[COMMON_CMD_LEN - 2])
	{
		return false;
	}

	/*判断设备是否正确*/
	if ((buffer[2] != network_local_device) && (buffer[2] != DEVICE_ALL))
	{
		return false;
	}

	/*判断设备是否冲突*/
	if (buffer[2] == buffer[1])
	{
		; // printf("device id repeat ID%d\n",buffer[2]);
	}
	return true;
}

typedef struct
{
	unsigned char send_device;
	unsigned char receive_device;
	unsigned char cmd;
	unsigned char arg1;
	unsigned char arg2;
} net_common_pack_info;

typedef struct
{
	const char *str;
	unsigned char cmd;
	void (*proc)(net_common_pack_info info);

	bool log_open;
} net_common_event_info;

bool device_online_state_get(network_device device)
{
	if (device == DEVICE_CCTV_1 || device == DEVICE_CCTV_2)
	{
		// Debug_Lib("%d,%s\n",device,device_heart_info[device_family_id][device].info.url);
		return device_heart_info[device_family_id][device].info.url[0] == 'r' ? true : false;
	}
	return device_heart_info[device_family_id][device].info.device_onlin_state;
}

bool family_dev_online_state_get(network_family family, network_device device)
{
	if (device == DEVICE_CCTV_1 || device == DEVICE_CCTV_2)
	{
		return device_heart_info[family][device].info.url[0] == 'r' ? true : false;
	}
	return device_heart_info[family][device].info.device_onlin_state;
}

bool device_enable_state_get(network_device device)
{
	// printf("%s =============>>>%d=%p\n",__func__,device,device_heart_info[device].enable);
	return device_heart_info[device_family_id][device].enable == NULL ? false : *(device_heart_info[device_family_id][device].enable);
}

void device_enable_state_set(network_device device, bool *enable)
{
	device_heart_info[device_family_id][device].enable = enable;
}

void device_cctv_url_set(network_device device, char *url)
{
	if (device != DEVICE_CCTV_1 && device != DEVICE_CCTV_2)
		device_heart_info[device_family_id][device].info.url = "";
	else
		device_heart_info[device_family_id][device].info.url = url;
}

bool device_repeat_state_get(void)
{
	return local_device_repeat;
}

bool tuya_monitor_state_get(void)
{
	return tuya_mon_state.tuya_monitor_ing;
}

static void net_common_deivce_repeat_func(net_common_pack_info info)
{
	// printf(PRINTF_RED "receive event:family_id :%d device id repeat ID%d\n" PRINTF_NONE,info.arg2, info.send_device);
	bool flag = info.arg1 ? true : false;
	if (flag)
	{
		ak_get_ostime(&tuya_mon_state.tuya_monitor_time);
		if (flag != tuya_mon_state.tuya_monitor_ing)
		{
			extern bool tuya_enter_monitor_push(void);
			tuya_enter_monitor_push();
			tuya_mon_state.tuya_monitor_ing = flag;
		}
	}
#if 0
	/*
	*	arg1: 1:收到查询ID状态请求
	*
	*		  2:收到ID状态
	*
	*			   arg2:最高位1表示id，冲突，低4位表示冲突的ID号
	*					最高位0:表示该id存在，但不冲突
	*/

	if(arg1 == 1)
	{
		/*收到查询id状态*/
		network_cmd_data data;
		data.device = device;
		data.cmd = NET_COMMON_CMD_ID_REPEAT;
		data.arg1 = 2;
	
		if(device == network_local_device && device_family_id == arg2)
		{
			
			data.arg2 = (0x80|(arg2 << 4))|network_local_device;
		}
		else
		{
			data.arg2  = (arg2 << 4)|network_local_device;
		}
		network_send_cmd_data(&data);
	}
	else if(arg1 == 2)
	{
		/*收到查询状态的结果*/
		if(device == DEVICE_OUTDOOR_1 || device == DEVICE_OUTDOOR_2)//收到来自户外机的应答
		{
			printf("receive outdoor response : %d\n",arg2&0x0F);
			net_online_device[arg2] = true;
		}
		else if(arg2&0x80)
		{
			printf("receive event:device id repeat ID%d0%d\n",(arg2>>4)&0x7,arg2&0x0F);
			extern bool device_id_repeat_push(network_device device);
			device_id_repeat_push(arg2);
		}
		else
		{
			printf("online device:{%x} \n",arg2);
			net_online_device[arg2] = true;
		}
	}
#else

	if (info.send_device == network_local_device && device_family_id == info.arg2) // 室内机ID冲突
	{
		// printf(PRINTF_RED "receive event:device_family_id :%d device id repeat ID%d\n" PRINTF_NONE,device_family_id, info.send_device);
		device_id_repeat_push(info.send_device);

		struct ak_timeval repeat_heart;
		ak_get_ostime(&repeat_heart);
		device_repeat_time = repeat_heart.sec; // 更新冲突心跳时间
		local_device_repeat = true;
	}
	else /*  if (info.send_device == DEVICE_OUTDOOR_1 || info.send_device == DEVICE_OUTDOOR_2 || device_family_id == info.arg2) */ // 发送方为门口机或者同户型室内机时接受处理心跳包
	{
		struct ak_timeval curr_time;
		ak_get_ostime(&curr_time);
		// if(curr_time.sec - device_heart_info[device_family_id][device].heart_time > BEATTIM)
		device_heart_info[info.arg2][info.send_device].heart_time = curr_time.sec;

		if (device_heart_info[info.arg2][info.send_device].info.device_onlin_state != true)
		{
			device_heart_info[info.arg2][info.send_device].info.device_onlin_state = true;
			printf(PRINTF_GREEN "receive from family[%d] deivce[%d] heart packet ........\n\r" PRINTF_NONE, info.arg2, info.send_device);
		}
		// fflush(stdout);
	}
#endif
}

static void net_common_outdoor_call_func(net_common_pack_info info)
{
	extern bool upgradeing_flag;
	if (upgradeing_flag)
		return;

	printf("======net_common_outdoor_call_func= %d ======>>>\n\n", info.arg2);
	printf("======family_id= %d ======>>>\n\n", device_family_id);
	fflush(stdout);
	if (info.arg2 != device_family_id)
	{
		printf("outdoor_call family error\n\n");
		return;
	}
	extern bool outdoor_call_event_push(char, char);
	outdoor_call_event_push(info.send_device, info.arg1);
}

static void net_common_motion_detect_func(net_common_pack_info info)
{
	extern bool motion_detect_event_push(char, char);
	motion_detect_event_push(info.send_device, info.arg1);
}

static void net_common_outdoor_talk_func(net_common_pack_info info)
{
	/*
	 * 室内机接收户外机通话前，需要知道是否已经有设备正在通话
	 *
	 * 如果没有，则发送到主线程（如果在监控页面，需要退出监控）
	 *
	 * 如果设备正在通话，则通知设备，设备正忙。
	 */
	MONITOR_CH monitor_ch = monitor_channel_get();
	MONITOR_CH talk_ch = (MONITOR_CH)info.arg1;
	if ((talk_ch != MON_CH_DOOR_1) && (talk_ch != MON_CH_DOOR_2))
	{
		printf("out door talk Parameter error %d \n", info.arg1);
		return;
	}
	printf("out door talk tlaking %d     monitor_ch%d      open:%d\n", info.arg1, monitor_ch, is_audio_talk_open());
	network_device outdoor_device = talk_ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2;
	if ((is_audio_talk_open() == AI_AO_O) && (monitor_ch == talk_ch))
	{
		printf("out door talk tlaking %d \n", info.arg1);
		/*当前通话正在通话*/
		network_cmd_data data;
		data.cmd = NET_COMMON_CMD_DEVICE_BUSY;
		data.arg1 = 1;
		data.arg2 = outdoor_device;
		data.device = info.send_device;
		network_send_cmd_data(&data);
	}
	else if (tuya_client_num_get() == 0 && info.send_device != network_local_device)
	{
		extern bool indoor_cmd_event_push(unsigned long arg1, unsigned long arg2);
		unsigned long arg2 = ((info.send_device & 0xFF) << 8) | (outdoor_device);
		indoor_cmd_event_push(1 << 8, arg2);
	}
}

static void net_common_outdoor_hand_func(net_common_pack_info info)
{
	MONITOR_CH talk_ch = (MONITOR_CH)info.arg1;
	if ((talk_ch != MON_CH_DOOR_1) && (talk_ch != MON_CH_DOOR_2) && info.send_device != network_local_device)
	{
		printf("out door talk Parameter error %d \n", info.arg1);
		return;
	}
	// Debug_Lib("\n");
	extern bool indoor_cmd_event_push(unsigned long arg1, unsigned long arg2);
	unsigned long arg2 = info.arg1 == MON_CH_DOOR_1	  ? DEVICE_OUTDOOR_1
						 : info.arg1 == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
													  : 0;
	indoor_cmd_event_push(1 << 8, arg2);
}

static void net_common_interphone_call_func(net_common_pack_info info)
{
	printf("receive family:%d    receive device:%d    send family:%d    send device:%d 	arg1 :%d\n", info.arg2 & 0x0F, info.receive_device, info.arg2 >> 4, info.send_device, info.arg1);
	if ((info.arg2 & 0x0F) != device_family_id)
	{
		return;
	}
	info.arg1 |= info.send_device << 4;
	extern void interphone_call_event_push(unsigned char, unsigned char);
	interphone_call_event_push(info.arg1, info.arg2);
}

void request_send_I_frame_cmd(network_device ch)
{
	Debug_Lib("========================================!\n");
	network_cmd_data data;
	data.cmd = NET_COMMON_CMD_STREAM_STATUS;
	data.arg1 = 0;
	data.arg2 = ch == DEVICE_OUTDOOR_1 ? monitor_config_get()->outdoor1->out_talk_volume : monitor_config_get()->outdoor2->out_talk_volume;
	data.device = ch;
	network_send_cmd_data(&data);
}

void add_del_card_cmd(network_device ch, char mode, char lock)
{
	network_cmd_data data;
	data.cmd = NET_COMON_CMD_ADD_DEL_CARD;
	data.arg1 = mode;
	data.arg2 = lock;
	data.device = ch;
	network_send_cmd_data(&data);
}

void def_unlock_time_cmd(network_device device, door_info door_temp)
{
	network_cmd_data data;
	data.arg1 = door_temp.unlock_delay;
	data.arg2 = door_temp.ungate1_delay;
	data.cmd = NET_COMMON_CMD_DEF_UNLOCK_TIME;
	data.device = device;
	Debug_Lib("@%d,%d,%d\n", device, door_temp.unlock_delay, door_temp.ungate1_delay);
	network_send_cmd_data(&data);
}

void def_exit_button_cmd(network_device device, bool lock_en, bool gate1_en)
{
	network_cmd_data data;
	data.arg1 = lock_en;
	data.arg2 = gate1_en;
	data.cmd = NET_COMMON_CMD_EXIT_BUTTON_TIME;
	data.device = device;
	network_send_cmd_data(&data);
}

void outdoor_reset_cmd(network_device ch)
{
	network_cmd_data data;
	data.cmd = NET_COMON_CMD_OUTDOOR_RESET;
	data.arg1 = 0;
	data.arg2 = 0;
	data.device = ch;
	network_send_cmd_data(&data);
}
static void net_common_compile_time_func(net_common_pack_info info)
{
	// printf("%s =======================>>>device :%d \n",__func__,device);

	struct ak_timeval curr_time;
	ak_get_ostime(&curr_time);
	device_heart_info[device_family_id][info.send_device].heart_time = curr_time.sec;

	if (OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].model != OLD_OUTDOOR_MODEL)
	{
		int ver = (info.arg2 << 8) | info.arg1;
		if (OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].ver != ver)
		{
			printf("new outdoor device:%d  version:%d\n", info.send_device, ver);
			OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].ver = ver;
			dev_info_status_event_push(1, 0);
		}
	}
	// printf("%d.%d.%d \n",year,mon,day);
}

static void net_common_def_unlock_time_func(net_common_pack_info info)
{
	struct ak_timeval curr_time;
	ak_get_ostime(&curr_time);
	device_heart_info[device_family_id][info.send_device].heart_time = curr_time.sec;
	// Debug_Lib("@,%d,%d\n",info.arg1,info.arg2);
	if (info.send_device == DEVICE_OUTDOOR_1)
	{
		moniotr_conf.outdoor1->unlock_delay = info.arg1;
		moniotr_conf.outdoor1->ungate1_delay = info.arg2;
	}
	else if (info.send_device == DEVICE_OUTDOOR_2)
	{
		moniotr_conf.outdoor2->unlock_delay = info.arg1;
		moniotr_conf.outdoor2->ungate1_delay = info.arg2;
	}
}

static void net_common_def_exit_button_func(net_common_pack_info info)
{
	// Debug_Lib("%d,%d\n", info.arg1, info.arg2);
	struct ak_timeval curr_time;
	ak_get_ostime(&curr_time);
	device_heart_info[device_family_id][info.send_device].heart_time = curr_time.sec;
	if (info.send_device == DEVICE_OUTDOOR_1)
	{
		moniotr_conf.outdoor1->exit_button_lock = info.arg1;
		moniotr_conf.outdoor1->exit_button_gate1 = info.arg2;
	}
	else if (info.send_device == DEVICE_OUTDOOR_2)
	{
		moniotr_conf.outdoor2->exit_button_lock = info.arg1;
		moniotr_conf.outdoor2->exit_button_gate1 = info.arg2;
	}
}

static void net_common_stream_status_func(net_common_pack_info info)
{
	extern bool get_video_data_display_state(void);
	bool monitor_message_ing(void);

	MONITOR_CH ch = monitor_channel_get();

	// Debug_Lib("ch:%d,arg1 :0x%x\n\r\n\r\n\r\n\r", ch, info.arg1);
	struct ak_timeval curr_time;
	ak_get_ostime(&curr_time);
	// if(curr_time.sec - device_heart_info[device_family_id][device].heart_time > BEATTIM)
	device_heart_info[device_family_id][info.send_device].heart_time = curr_time.sec;

	if (device_heart_info[device_family_id][info.send_device].info.device_onlin_state == false)
	{
		printf(PRINTF_PURPLE " device %d pop up online ..........\n\r" PRINTF_NONE, info.send_device);
		dev_info_status_event_push(1, 0);
		device_heart_info[device_family_id][info.send_device].info.device_onlin_state = true;
	}
	// printf(PRINTF_GREEN"receive from outdoor[%d] heart packet ........\n\r"PRINTF_NONE,device == DEVICE_OUTDOOR_1 ? 1 : 2);
	// fflush(stdout);
	// printf("OUTDOOR VERSION: %d.%d \n",arg1 >> 4,arg2);
	if (info.arg1 & 0xF0)
	{
		OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].model = OLD_OUTDOOR_MODEL;
		int ver = (info.arg1 >> 4) * 100 + info.arg2;
		if (OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].ver != ver)
		{
			printf("old outdoor device:%d  version:%d\n", info.send_device, ver);
			OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].ver = ver;
		}
	}
	else
	{
		OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].model = info.arg2;
	}
	OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].talk_busy = info.arg1 & 0x02;
	OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].fingerprint_module = info.arg1 & 0x04;
	if (OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].talk_busy)
	{
		extern bool device_monitor_busy_push(unsigned char arg1, unsigned char arg2);
		device_monitor_busy_push(info.send_device, 0);
	}
	// printf(PRINTF_GREEN"info.arg1:%d,receive from outdoor[%d] fingerprint_module[%d]........\n\r"PRINTF_NONE,info.arg1,info.send_device == DEVICE_OUTDOOR_1 ? 1 : 2,OutDoor_Info.info[info.send_device - DEVICE_OUTDOOR_1].fingerprint_module);
	/*
		data.arg1:
				  bit[0]:视频流稳定标志
				  bit[1]:留言开启标志
				  bit[2]:留言语言类型

		data.arg2:门口机回铃声设置，仅由设备1作唯一参考，其余设备参数无效且固定为0，若设备1关闭回铃声功能或手动进入监控，即参数为0
				  bit[0:4]:回铃声音量大小
				  bit[5:8]:铃声序号
	*/
	// printf("0x%x ,0x%x,curr_network_video_receive_eth_id_get() :0x%x\n",network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()),0),network_common_socket_eth_p_get(0, network_get_id_outdoor2(network_local_device_get()),0),curr_network_video_receive_eth_id_get());
	if ((ch == MON_CH_DOOR_1) && (info.send_device == DEVICE_OUTDOOR_1))
	{
		if (network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()), 0) == curr_network_video_receive_eth_id_get())
		{
			ak_thread_mutex_lock(&outdoor_order_mutex);
			network_cmd_data data;
			data.cmd = NET_COMMON_CMD_STREAM_STATUS;
			data.arg1 = (tuya_client_num_get() ? true : get_video_data_display_state()) | outdoor_order_arg2;
			data.arg2 = monitor_config_get()->outdoor1->out_talk_volume;
			data.device = DEVICE_OUTDOOR_1;
			network_send_cmd_data(&data);
			//    printf("send NET_COMMON_CMD_STREAM_STATUS -> OUTDOOR1  outdoor_order_arg2 : %d \n",outdoor_order_arg2);

			ak_thread_mutex_unlock(&outdoor_order_mutex);
		}
	}
	else if (monitor_config_get()->outdoor2->enable_sw && (ch == MON_CH_DOOR_2) && (info.send_device == DEVICE_OUTDOOR_2))
	{
		if (network_common_socket_eth_p_get(0, network_get_id_outdoor2(network_local_device_get()), 0) == curr_network_video_receive_eth_id_get())
		{
			ak_thread_mutex_lock(&outdoor_order_mutex);
			network_cmd_data data;
			data.cmd = NET_COMMON_CMD_STREAM_STATUS;
			data.arg1 = (tuya_client_num_get() ? true : get_video_data_display_state()) | outdoor_order_arg2;
			data.arg2 = monitor_config_get()->outdoor2->out_talk_volume;
			data.device = DEVICE_OUTDOOR_2;
			network_send_cmd_data(&data);
			//  printf("send NET_COMMON_CMD_STREAM_STATUS -> OUTDOOR2 \n");

			ak_thread_mutex_unlock(&outdoor_order_mutex);
		}
	}
	else if (network_local_device_get() == DEVICE_INDOOR_ID1 && (info.arg1 & 0x01) == 1) // 接受到移动侦测信号且未进入视频通道
	{
		// printf(PRINTF_YELLOW"RECEIVE MOTION DETECT SIGNAL .........%d\n\r"PRINTF_NONE,__LINE__);
		if (info.send_device == DEVICE_OUTDOOR_1 && !(monitor_config_get()->outdoor1->motion_sensitivity && device_online_state_get(DEVICE_OUTDOOR_1)))
		{
			return;
		}
		else if (info.send_device == DEVICE_OUTDOOR_2 && (!monitor_config_get()->outdoor2->enable_sw || !(monitor_config_get()->outdoor2->motion_sensitivity && device_online_state_get(DEVICE_OUTDOOR_2))))
		{
			return;
		}
		extern bool motion_detect_event_push(char, char);
		motion_detect_event_push(info.send_device, info.arg1 & 0x01);
	}

	// printf(PRINTF_GREEN"receive from deivce[%d] heart packet ........\n\r"PRINTF_NONE,info.arg1 & 0x01);
}

static void net_common_mailbox_status_func(net_common_pack_info info)
{
	extern bool mailbox_status_detect_push(void);
	if (info.send_device == DEVICE_OUTDOOR_1 && monitor_config_get()->outdoor1->mailbox_num != info.arg1)
	{
		printf(PRINTF_GREEN "door1_mailbox_status_func[%d] heart packet ........\n\r", info.arg1);
		monitor_config_get()->outdoor1->mailbox_num = info.arg1;
		mailbox_status_detect_push();
		return;
	}
	else if (info.send_device == DEVICE_OUTDOOR_2 && monitor_config_get()->outdoor2->mailbox_num != info.arg1)
	{
		printf(PRINTF_GREEN "door2_mailbox_status_func[%d] heart packet ........\n\r", info.arg1);
		monitor_config_get()->outdoor2->mailbox_num = info.arg1;
		mailbox_status_detect_push();
		return;
	}
}

static void net_common_device_busy_func(net_common_pack_info info)
{
	extern bool indoor_cmd_event_push(unsigned long arg1, unsigned long arg2);
	unsigned long arg2_1 = ((info.send_device & 0xFF) << 8) | (info.arg2);
	indoor_cmd_event_push((2 << 8) | info.arg1, arg2_1);
}

static void net_common_gate2_unlock_func(net_common_pack_info info)
{
	extern void device_gate2_unlock_push(unsigned long arg1, unsigned long arg2);
	device_gate2_unlock_push(0, 0);
}

volatile int check_flag = 0;
bool upgradeing_flag = false;

static int check_time = 0;
extern bool network_upgrade_send_package_open(void);
extern bool network_upgrade_sent_package_close(void);
// arg1 == 1 升级完成 arg2 == 2 失败 重新发送
static void net_common_upgrade_reset_func(net_common_pack_info info)
{
	if (!upgradeing_flag)
	{
		return;
	}
	if (info.arg1 == 1)
	{ // 设备在线
		extern bool upgrade_event_push(char, char);
		extern void upgrade_pack_len_set(int len);
		upgrade_pack_len_set(UPGRADE_PACK_LEN + UPGRADE_PACK_LEN * info.arg2);

		if (access("/mnt/tf/cbin.update", F_OK) != 0)
		{
			// 文件不存在
			upgrade_event_push(6, 0);
			return;
		}

		upgrade_event_push(1, info.send_device);
	}
	else if (info.arg1 == 2)
	{

		printf("outdoor upgrade over!!!\n");
		fflush(stdout);
		// 升级成功，删除窗口
		extern bool upgrade_event_push(char, char);
		upgrade_event_push(2, 0);
	}
	else if (info.arg1 == 3)
	{ // 升级失败 重新发送
		printf("device :%d outdoor recive error sent again!!!\n", info.send_device);
		extern bool upgrade_event_push(char, char);
		upgrade_event_push(7, 0);
	}
}

static net_common_event_info net_common_event[] =
	{
		{"device id repeat", NET_COMMON_CMD_ID_REPEAT, net_common_deivce_repeat_func, false},
		{"out door call", NET_COMMON_CMD_OUTDOOR_CALL, net_common_outdoor_call_func, false},
		{"indoor and outdoor talk", NET_COMMON_CMD_OUTDOOR_TALK, net_common_outdoor_talk_func, false},
		{"indoor and outdoor hand", NET_COMMON_CMD_OUTDOOR_HANG, net_common_outdoor_hand_func, false},
		{"interphone call ", NET_COMMON_CMD_INTERCOM_CALL, net_common_interphone_call_func, true},
		{"stream status empty", NET_COMMON_CMD_STREAM_STATUS, net_common_stream_status_func, false},
		{"Compile-time packet", NET_COMMON_CMD_COMPILE_TIME, net_common_compile_time_func, false},
		{"Default unlock-time packet", NET_COMMON_CMD_DEF_UNLOCK_TIME, net_common_def_unlock_time_func, false},
		{"Default exit-button lock packet", NET_COMMON_CMD_EXIT_BUTTON_TIME, net_common_def_exit_button_func, false},

		{"mailbox status get", NET_COMMON_CMD_MAILBOX_STATUS, net_common_mailbox_status_func, false},
		{"device busy", NET_COMMON_CMD_DEVICE_BUSY, net_common_device_busy_func, true},
		{"motion detect", NET_COMMON_CMD_MOTION, net_common_motion_detect_func, true},
		{"Indoor gate2 unlock", NET_COMMON_CMD_GATE2_UNLOCK, net_common_gate2_unlock_func, true},
		{"outdoor upgrade reset", NET_COMON_CMD_UPGRADE_OUTDOOR, net_common_upgrade_reset_func, false},
};

static bool net_common_event_process(net_common_pack_info info)
{
	int size = sizeof(net_common_event) / sizeof(net_common_event_info);
	for (int i = 0; i < size; i++)
	{
		if (net_common_event[i].cmd == info.cmd)
		{
			if (net_common_event[i].log_open == true)
			{
				printf("receive cmd %s device:%d\n", net_common_event[i].str, info.send_device);
			}
			net_common_event[i].proc(info);
			break;
		}
	}
	return true;
}

static void *network_cmd_receive_task(void *arg)
{
	fd_set readfds;
	struct timeval timeout;
	struct ak_timeval curr_time;
	unsigned long pre_sec = 0;
	int heart_check_time = 0;
	char buffer[512] = {0};
	while (1)
	{
		FD_ZERO(&readfds);
		FD_SET(cmd_receive_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;
		int ret_select = select(cmd_receive_fd + 1, &readfds, NULL, NULL, &timeout);
		if (ret_select > 0)
		{
			if (FD_ISSET(cmd_receive_fd, &readfds))
			{
				int read_len = recv(cmd_receive_fd, buffer, sizeof(buffer), 0);
				// int read_len = recvfrom(cmd_receive_fd, buffer, sizeof(buffer), 0, NULL, NULL);
				// printf("receive size:%d \n",read_len);
				if ((read_len == 68) && (buffer[12] == ((ETH_P_CMD >> 8) & 0xFF)) && (buffer[13] == (ETH_P_CMD & 0xFF)) && (net_common_code_check_valid(&buffer[60])))
				{
					net_common_pack_info info = {0};
					memcpy(&info, &buffer[61], sizeof(net_common_pack_info));
					net_common_event_process(info);
				}
			}
		}

		ak_get_ostime(&curr_time);

		if (curr_time.sec != pre_sec)
		{
			pre_sec = curr_time.sec;

			if (tuya_mon_state.tuya_monitor_ing)
			{
				if (ak_diff_ms_time(&curr_time, &tuya_mon_state.tuya_monitor_time) > 3000)
				{
					tuya_mon_state.tuya_monitor_ing = false;
				}
			}

			if (check_flag == 1)
			{
				printf("check outdoor online!!!!!!====:%d\n", check_time);
				if (++check_time > 2)
				{
					// 设备离线
					extern bool upgrade_event_push(char, char);
					upgrade_event_push(3, 0);
					check_time = 0;
					check_flag = 0;
				}
			}

			/* 检查冲突是否持续 */
			if (local_device_repeat == true && (curr_time.sec - device_repeat_time > (BEATTIM / 2)))
			{
				local_device_repeat = false;
				device_id_repeat_push(DEVICE_UNKONW);
			}

			/*每隔五秒检查心跳包*/
			heart_check_time++;
			if (heart_check_time > BEATTIM)
			{
				heart_check_time = 0;
				for (network_family family = FAMILY_ID1; family < FAMILY_TOTAL; family++)
				{
					for (network_device dev = DEVICE_INDOOR_ID1; dev < DEVICE_CCTV_1; dev++)
					{
						// if(device_heart_info[family][dev].info.device_onlin_state == true && dev < DEVICE_OUTDOOR_1){
						// 	Debug_Lib( "family[%d] device %d heat time:%lu curr:%lu \n\r" PRINTF_NONE,family, dev,device_heart_info[family][dev].heart_time,curr_time.sec);
						// }

						if ((device_heart_info[family][dev].info.device_onlin_state == true) && (curr_time.sec - device_heart_info[family][dev].heart_time > BEATTIM) /*  && (dev != network_local_device) */) // 超五秒未收到心跳包
						{
							device_heart_info[family][dev].info.device_onlin_state = false;
							if (dev == DEVICE_OUTDOOR_1 || dev == DEVICE_OUTDOOR_2)
							{
								OutDoor_Info.info[dev - DEVICE_OUTDOOR_1].model = 0;
								set_outdoor_version(dev, 0);
								dev_info_status_event_push(1, 0);
								OutDoor_Info.info[0].talk_busy = OutDoor_Info.info[1].talk_busy = 0;
							}
							Debug_Lib(PRINTF_PURPLE "family[%d] device %d off line ..........\n\r" PRINTF_NONE, family, dev);
						}
					}
				}
			}

			/*每隔一秒发送心跳包*/
			network_cmd_data data;
			data.device = DEVICE_ALL;
			data.cmd = NET_COMMON_CMD_ID_REPEAT;
			data.arg1 = monitor_enter_way_get() == MONITOR_ENTER_TUYA && !tuya_monitor_state_get() ? true : false;
			data.arg2 = device_family_id;
			network_send_cmd_data(&data);
			// printf(PRINTF_YELLOW"sned heart to DEVICE_ALL ...........%d\n\r"PRINTF_NONE,data.arg1);

			// system("sync");
		}
		// ak_sleep_ms(1);
	}

	ak_thread_exit();
	return NULL;
}

bool network_init(network_device device)
{
	ak_thread_mutex_init(&network_device_mutex, NULL);
	ak_thread_mutex_init(&outdoor_order_mutex, NULL);
	network_device_init(device);
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_cmd_receive_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

const char *nework_get_package_head(int type)
{
	static char *mac_local = NULL;
	if (mac_local == NULL)
	{
		struct ifreq req;
		int fd = socket(PF_INET, SOCK_DGRAM, 0);
		strcpy(req.ifr_name, NETWORK_NAME);
		ioctl(fd, SIOCGIFHWADDR, &req);
		close(fd);

		mac_local = ak_mem_alloc(MODULE_ID_APP, 60);
		memset(mac_local, 0, 60);
#if 1
		// 设置目的网卡地址
		mac_local[0] = 0x01;
		mac_local[1] = 0x01;
		mac_local[2] = 0x01;
		mac_local[3] = 0x01;
		mac_local[4] = 0x01;
		mac_local[5] = 0x01;
#endif
		memcpy(&mac_local[6], req.ifr_hwaddr.sa_data, 6);
		// mac_local[12] = 0x88;
		printf("================>>> %s:%d \n", __func__, __LINE__);
	}
	mac_local[12] = type / 256;
	mac_local[13] = type % 256;
	return mac_local;
}

static void network_cmd_code_get(network_cmd_data *src, char *dst)
{
	network_cmd_data *device = (network_cmd_data *)src;

	dst[0] = NET_COMMON_CMD_START;
	dst[1] = network_local_device;
	dst[2] = device->device;
	dst[3] = device->cmd;
	dst[4] = device->arg1;
	dst[5] = device->arg2;
	dst[6] = (dst[1] + dst[2] + dst[3] + dst[4] + dst[5]) & 0xFF;
	dst[7] = NET_COMMON_CMD_END;
}

bool network_send_cmd_data(network_cmd_data *data)
{
	if (data == NULL)
	{
		return true;
	}
	char package[68];
	memcpy(package, nework_get_package_head(ETH_P_CMD), 60);
	network_cmd_code_get(data, &package[60]);

	ak_thread_mutex_lock(&network_device_mutex);
	if (sendto(cmd_send_fd, package, sizeof(package), 0, (struct sockaddr *)&nework_local_sockaddr_ll, sizeof(nework_local_sockaddr_ll)) < 0)
	{
		perror("sendto fail ");
	}
	ak_thread_mutex_unlock(&network_device_mutex);
	ak_sleep_ms(1);
	return true;
}

extern int upgrade_pack_len_get(void);
static void network_upgradecmd_code_get(network_upgradecmd_data *src, char *dst)
{
	network_upgradecmd_data *device = (network_upgradecmd_data *)src;

	dst[0] = NET_COMMON_CMD_START;
	dst[1] = network_local_device;
	dst[2] = device->device;
	dst[3] = device->cmd;

	dst[4] = (device->arg1 >> 24) & 0xFF;
	dst[5] = (device->arg1 >> 16) & 0xFF;
	dst[6] = (device->arg1 >> 8) & 0xFF;
	dst[7] = device->arg1 & 0xFF;

	dst[8] = (device->arg2 >> 24) & 0xFF;
	dst[9] = (device->arg2 >> 16) & 0xFF;
	dst[10] = (device->arg2 >> 8) & 0xFF;
	dst[11] = device->arg2 & 0xFF;

	memcpy(&dst[12], device->buf, upgrade_pack_len_get());

	dst[upgrade_pack_len_get() + 12] = NET_COMMON_CMD_END;
}

bool network_sendupgrade_cmd_data(network_upgradecmd_data *data)
{
	char package[(UPGRADE_PACK_LEN * 2) + 73] = {0};
	memcpy(package, nework_get_package_head(ETH_P_CMD), 60); // 包头
	network_upgradecmd_code_get(data, &package[60]);		 // 完成封包

	ak_thread_mutex_lock(&network_device_mutex);
	if (sendto(cmd_send_fd, package, upgrade_pack_len_get() + 73, 0, (struct sockaddr *)&nework_local_sockaddr_ll, sizeof(nework_local_sockaddr_ll)) < 0)
	{
		perror("sendto fail ");
	}
	ak_thread_mutex_unlock(&network_device_mutex);
	ak_sleep_ms(1);
	return true;
}

int network_common_socket_eth_p_audio_get(int slave_id, int family)
{
	printf("family:%d   device_family_id:%d\n", family, device_family_id);
	int SOCKET_ETH_P_BASE_AUDIO = device_family_id;

	if (family < device_family_id)
	{
		SOCKET_ETH_P_BASE_AUDIO = (device_family_id << 12) | (family << 8);
	}
	else
	{
		SOCKET_ETH_P_BASE_AUDIO = (family << 12) | (device_family_id << 8);
	}

	int mstart_id = 0;

	mstart_id = network_get_id_slave_id(network_local_device, network_local_device);

	if ((slave_id == network_get_id_outdoor1(network_local_device)) || (slave_id == network_get_id_outdoor2(network_local_device)))
	{
		SOCKET_ETH_P_BASE_AUDIO = 0x2600;
		mstart_id = slave_id;
	}

	int vol = 0;
	printf("==============>>mstart_id【0x%x】\n==============>>slave_id【0x%x】\n==============>>network_local_device 【%d】\n", mstart_id, slave_id, network_local_device);
	if (mstart_id < slave_id)
	{
		vol = (slave_id << 4) | mstart_id;
	}
	else
	{
		vol = (mstart_id << 4) | slave_id;
	}
	return SOCKET_ETH_P_BASE_AUDIO | vol;
}

int network_common_socket_eth_p_video_get(int slave_id)
{
#define SOCKET_ETH_P_BASE_VIDEO 0X1600
	return SOCKET_ETH_P_BASE_VIDEO | slave_id;
}

int network_common_socket_eth_p_get(char type, int slave_id, int family)
{
	/*
	 *type 1:代表音频，0:代表视频
	 */
	if (type == 1)
	{
		return network_common_socket_eth_p_audio_get(slave_id, family);
	}
	else if (type == 0)
	{
		return network_common_socket_eth_p_video_get(slave_id);
	}
	return 0;
}
