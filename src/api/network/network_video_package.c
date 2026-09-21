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
#include "queue.h"
#include "leo_api.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "ak_common.h"
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "ak_common_video.h"
#include "file_api.h"
#include "video_decode.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_sdk.h"
#include "tuya_ipc_cloud_storage.h"

#define VIDEO_QUEUE_PACKAGE_MAX 64

#define VIDEO_PACKAGE_SIZE_MAX (32 * 1024) // 1500 //10*1024//1510 //32

#define VIDEO_FRAME_MAX (512 * 1024)

#define VIDEO_IP_ADDRES "255.255.255.255" //"192.168.37.1"//

static int network_video_send_eth_id = 0;

static const char video_start_code[4] = {0x00, 0x00, 0x01, 0xfc};

extern struct sockaddr_ll *network_get_send_addres(void);
extern const char *nework_get_package_head(unsigned int type);
extern bool video_decode_push(char, unsigned char *data, int len);
extern bool video_decode_queue_reset(void);
extern bool video_record_data_push(record_data_node *node);

typedef struct
{
	char frame_type; // 0:h264,1mjpeg
	char *data;
	int len;
	unsigned long long pts;
	unsigned long long frame_index;
} network_video_data;

typedef struct
{
	void *prev;
	void *next;

	network_video_data package;
} network_video_package;
static queue_s network_video_send_queue_free;
static queue_s network_video_send_queue_head;

static ak_mutex_t network_video_send_head_mutex;
static ak_mutex_t network_video_send_free_mutex;

static network_video_package network_video_package_buffer[VIDEO_QUEUE_PACKAGE_MAX];
static void network_video_send_package_queue_init(void)
{
	static bool is_first = true;
	if (is_first == false)
	{
		return;
	}
	is_first = false;

	ak_thread_mutex_init(&network_video_send_head_mutex, NULL);
	ak_thread_mutex_init(&network_video_send_free_mutex, NULL);

	queue_initialize(&network_video_send_queue_head);
	queue_initialize(&network_video_send_queue_free);
	memset(network_video_package_buffer, 0, sizeof(network_video_package) * VIDEO_QUEUE_PACKAGE_MAX);
	for (int i = 0; i < VIDEO_QUEUE_PACKAGE_MAX; i++)
	{
		queue_insert((queue_s *)&network_video_package_buffer[i], &network_video_send_queue_free);
	}
}

static network_video_package *network_video_send_package_queue_new(char type, const char *data, int size)
{
	network_video_package *node = NULL;
	ak_thread_mutex_lock(&network_video_send_free_mutex);
	if (queue_empty(&network_video_send_queue_free) == 0)
	{
		node = (network_video_package *)queue_delete_next(&network_video_send_queue_free);
	}
	ak_thread_mutex_unlock(&network_video_send_free_mutex);
	if (node == NULL)
	{
		return NULL;
	}

	if (node->package.data != NULL)
	{
		ak_mem_free(node->package.data);
	}
	node->package.data = (char *)ak_mem_alloc(MODULE_ID_APP, size);
	memcpy(node->package.data, data, size);
	static unsigned long long frame_index = 0;
	node->package.frame_index = frame_index++;
	node->package.frame_type = type;
	node->package.len = size;
	node->package.pts = get_sys_ms();
	// printf("send index :%llu \n",node->package.frame_index);
	return node;
}
static void network_video_send_package_queue_del(network_video_package *node)
{
	if (node != NULL)
	{
		if (node->package.data != NULL)
		{
			ak_mem_free(node->package.data);
			node->package.data = NULL;
		}

		ak_thread_mutex_lock(&network_video_send_free_mutex);
		queue_insert((queue_s *)node, &network_video_send_queue_free);
		ak_thread_mutex_unlock(&network_video_send_free_mutex);
	}
}

static void network_video_send_release_all(void)
{
	ak_thread_mutex_lock(&network_video_send_head_mutex);
	while (!queue_empty(&network_video_send_queue_head))
	{
		network_video_package *node = (network_video_package *)queue_delete_next(&network_video_send_queue_head);
		network_video_send_package_queue_del(node);
	}
	ak_thread_mutex_unlock(&network_video_send_head_mutex);
}

static bool network_video_send_thread_run = false;
static bool networK_video_send_task_run = false;
static bool network_video_send_ready = false;
static int video_package_send_fd = -1;
static bool network_video_send_socket_open(void)
{
	if (video_package_send_fd != -1)
	{
		return false;
	}

	printf("==========>>> video send socket %04x <<<==========\n", network_video_send_eth_id);
#if 0
	if((video_package_send_fd = socket(PF_PACKET, SOCK_RAW, htons(network_video_send_eth_id))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}
#endif
	if ((video_package_send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("create socket error raw_socket_receive_fd\n");
		return false;
	}

	struct ifreq interface;
	memset(&interface, 0x00, sizeof(interface));
	snprintf(interface.ifr_name, IFNAMSIZ, "%s", NETWORK_NAME);
	if (setsockopt(video_package_send_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
	{
		perror("eth1-client:SO_BINDTODEVICEfailed");
		return false;
	}

	int on = 1;
	setsockopt(video_package_send_fd, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &on, sizeof(on));
	return true;
}

static bool network_video_send_socket_close(void)
{
	if (video_package_send_fd == -1)
	{
		return false;
	}

	close(video_package_send_fd);
	video_package_send_fd = -1;

	return true;
}

static void network_send_package_video(network_video_package *node)
{
	network_video_data *package = &(node->package);
	int send_size = 0, remain_size = package->len;
	char *buffer = (char *)ak_mem_alloc(MODULE_ID_APP, VIDEO_PACKAGE_SIZE_MAX);
	memset(buffer, 0, VIDEO_PACKAGE_SIZE_MAX);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(VIDEO_IP_ADDRES);
	serv_addr.sin_port = htons(network_video_send_eth_id); // network_video_send_eth_id

	while (remain_size > 0)
	{
		// memcpy(&buffer[0],nework_get_package_head(network_video_send_eth_id),60);
		if (send_size == 0)
		{
			memcpy(&buffer[0], video_start_code, 4);
			buffer[4] = (remain_size >> 24) & 0xFF;
			buffer[5] = (remain_size >> 16) & 0xFF;
			buffer[6] = (remain_size >> 8) & 0xFF;
			buffer[7] = remain_size & 0xFF;

			buffer[8] = (package->pts >> 24) & 0xFF;
			buffer[9] = (package->pts >> 16) & 0xFF;
			buffer[10] = (package->pts >> 8) & 0xFF;
			buffer[11] = package->pts & 0xFF;

			buffer[12] = (package->frame_index >> 24) & 0xFF;
			buffer[13] = (package->frame_index >> 16) & 0xFF;
			buffer[14] = (package->frame_index >> 8) & 0xFF;
			buffer[15] = package->frame_index & 0xFF;
			buffer[16] = package->frame_type;

			if (remain_size > (VIDEO_PACKAGE_SIZE_MAX - 17))
			{
				memcpy(&buffer[17], &package->data[send_size], VIDEO_PACKAGE_SIZE_MAX - 17);
				// write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
				if (sendto(video_package_send_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
				{
					perror(" send to fail \n");
				}
				remain_size -= (VIDEO_PACKAGE_SIZE_MAX - 17);
				send_size += (VIDEO_PACKAGE_SIZE_MAX - 17);
			}
			else
			{
				memcpy(&buffer[17], &package->data[send_size], remain_size);
				// write(file_fd,buffer,remain_size + 31);
				if (sendto(video_package_send_fd, buffer, remain_size + 17, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
				{
					perror(" send to fail \n");
				}
				break;
			}
		}

		// ak_sleep_ms(1);
		if (remain_size > VIDEO_PACKAGE_SIZE_MAX)
		{
			memcpy(&buffer[0], &package->data[send_size], VIDEO_PACKAGE_SIZE_MAX - 0);
			//	write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
			if (sendto(video_package_send_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			remain_size -= (VIDEO_PACKAGE_SIZE_MAX);
			send_size += (VIDEO_PACKAGE_SIZE_MAX);
		}
		else
		{
			memcpy(&buffer[0], &package->data[send_size], remain_size);
			// write(file_fd,buffer,remain_size + 14);
			if (sendto(video_package_send_fd, buffer, remain_size + 0, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			break;
		}
	}
	ak_mem_free(buffer);
}

static void *network_video_send_package_task(void *arg)
{
	network_video_send_thread_run = true;

	network_video_send_socket_open();

	network_video_send_ready = true;
	while (networK_video_send_task_run == true)
	{
		network_video_package *node = NULL;
		ak_thread_mutex_lock(&network_video_send_head_mutex);
		if (queue_empty(&network_video_send_queue_head) == 0)
		{
			node = (network_video_package *)queue_delete_next(&network_video_send_queue_head);
		}
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		if (node != NULL)
		{
			network_send_package_video(node);
			network_video_send_package_queue_del(node);
		}
		ak_sleep_ms(1);
	}

	ak_thread_mutex_lock(&network_video_send_head_mutex);
	network_video_send_ready = false;
	ak_thread_mutex_unlock(&network_video_send_head_mutex);

	network_video_send_socket_close();
	network_video_send_release_all();
	network_video_send_thread_run = false;

	printf("===========<<< network video send finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_video_send_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_video_send_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

bool network_video_send_package_open(unsigned long id)
{
	network_video_send_package_queue_init();

	if (networK_video_send_task_run == true)
	{
		return false;
	}

	if (network_video_send_wait_thread_quit() == false)
	{
		return false;
	}

	network_video_send_eth_id = id;
	networK_video_send_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_video_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

bool network_video_send_package_close(void)
{
	if (networK_video_send_task_run == false)
	{
		return false;
	}
	networK_video_send_task_run = false;
	return true;
}

bool network_video_send_package_push(char type, const char *data, int len)
{
	ak_thread_mutex_lock(&network_video_send_head_mutex);
	if (network_video_send_ready == false)
	{
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		return false;
	}

	network_video_package *node = network_video_send_package_queue_new(type, data, len);
	if (node == NULL)
	{
		printf("network_video_send_package_push full\n\r");
		ak_thread_mutex_unlock(&network_video_send_head_mutex);
		return false;
	}
	queue_insert((queue_s *)node, &network_video_send_queue_head);
	ak_thread_mutex_unlock(&network_video_send_head_mutex);
	return true;
}

static int network_video_receive_eth_id = 0;
static bool network_video_receive_thread_run = false;
static bool networK_video_receive_task_run = false;
static int video_package_receive_fd = -1;

static bool network_video_receive_socket_open(int receive_eth_id)
{
	if (video_package_receive_fd != -1)
	{
		return false;
	}
	printf("==========>>> video receive socket %0x <<<==========\n", receive_eth_id);
#if 0 // PF_INET  htons(ETH_P_ALL )
	if((video_package_receive_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

    struct ifreq req;
    int fd = socket(PF_INET,SOCK_DGRAM,0);
    strcpy(req.ifr_name,NETWORK_NAME);
    ioctl(fd,SIOCGIFINDEX,&req);
    close(fd);


    struct sockaddr_ll sll;
    struct packet_mreq mr;
    memset( &sll, 0, sizeof( sll ) );
    sll.sll_family   = PF_PACKET;
    sll.sll_ifindex  = req.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_IP);//htons( ETH_P_ALL/*network_video_receive_eth_id*/  );
    if( bind( video_package_receive_fd, (struct sockaddr *) &sll, sizeof( sll ) ) == -1 )
    {
        perror( "bind" );
        return( 1 );
    }

    memset( &mr, 0, sizeof( mr ) );
    mr.mr_ifindex = req.ifr_ifindex;
    mr.mr_type    = PACKET_MR_PROMISC;
    if( setsockopt( video_package_receive_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP,&mr, sizeof( mr ) ) == -1 )
    {
        perror( "setsockopt" );
        return( 1 );
    }

    int on=1;
    if(setsockopt(video_package_send_fd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&on,sizeof(on)) < 0)
    {
        printf("setsockopt");
    }
#else
	if ((video_package_receive_fd = socket(AF_INET, /*SOCK_STREAM*/ SOCK_DGRAM, 0)) == -1)
	{
		perror("socket fail \n");
		return false;
	}

#if 1
	struct ifreq interface;
	memset(&interface, 0x00, sizeof(interface));
	snprintf(interface.ifr_name, IFNAMSIZ, "%s", NETWORK_NAME);
	if (setsockopt(video_package_receive_fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
	{
		printf("eth1-client:SO_BINDTODEVICEfailed");
		return false;
	}
#endif
	int on = 1; //|
	if (setsockopt(video_package_receive_fd, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &on, sizeof(on)) < 0)
	{
		printf("setsockopt 1\n\r");
	}

	if (setsockopt(video_package_receive_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) < 0)
	{
		printf("setsockopt 2\n\r");
	}

	// system("netstat -nau");
	// ak_sleep_ms(50);
	// printf("Query whether the current port is occupied!!!!!!!!!!!!!!!!!!!!\n");
#if 1
	struct sockaddr_in receive_addr;
	memset(&receive_addr, 0x00, sizeof(receive_addr));
	receive_addr.sin_family = AF_INET;
	receive_addr.sin_port = htons(receive_eth_id);
	printf("receive_eth_id :%d\n\r", receive_eth_id);
	receive_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // VIDEO_IP_ADDRES
	if (bind(video_package_receive_fd, (struct sockaddr *)&receive_addr, sizeof(receive_addr)) == -1)
	{
		perror("bind socket error:\n");
		return false;
	}
#endif

	int recv_buf_size = 1024 * 1024 * 2; // 100 * 1024;
	if (setsockopt(video_package_receive_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(int)) < 0)
	{
		perror("setsockopt SO_RCVBUF\n");
		return false;
	}

	int opt_val;
	socklen_t opt_len = sizeof(opt_val);

	if (getsockopt(video_package_receive_fd, SOL_SOCKET, SO_RCVBUF, &opt_val, &opt_len) < 0)
	{
		perror("fail to getsockopt");
	}

	printf("SO_RCVBUF = %dk\n", opt_val / 1024);

#endif
	printf("create video receive socket success \n");
	return true;
}

static bool network_video_receive_socket_close(void)
{
	if (video_package_receive_fd == -1)
	{
		return false;
	}

	close(video_package_receive_fd);
	video_package_receive_fd = -1;

	return true;
}

typedef enum
{
	SS_IDLE,
	SS_ONGING,
	SS_STOP,
	SS_FINISH,
} tuya_ss_status;

extern unsigned long long os_get_ms(void);

static bool receive_frame_push = false;
extern OPERATE_RET ipc_app_sync_utc_time(VOID);
extern bool get_p2p_online_status(void);
static void *network_video_receive_package_task(void *arg)
{
	extern int h264_is_keyframe(const unsigned char *buffer, int len);
	fd_set readfds;
	struct timeval timeout;

	network_video_receive_thread_run = true;
	receive_frame_push = false;

	int receive_eth_id = *((int *)arg);
	network_video_receive_eth_id = receive_eth_id;
	extern void request_send_I_frame_cmd(network_device ch);
	if (network_common_socket_eth_p_get(0, network_get_id_outdoor1(network_local_device_get()), 0) == network_video_receive_eth_id)
	{
		request_send_I_frame_cmd(DEVICE_OUTDOOR_1);
	}
	else
	{
		request_send_I_frame_cmd(DEVICE_OUTDOOR_2);
	}
	network_video_receive_socket_open(receive_eth_id);

	// bool ss_start_event = monitor_channel_get() ==  MON_CH_DOOR_1 ?  (monitor_config_get()->outdoor1->motion_duration >= 30 ? true :  false) : (monitor_config_get()->outdoor2->motion_duration >= 30 ? true :  false);
	tuya_ss_status ss_event_status = SS_IDLE;
	// unsigned long long receive_frame_pts = 0;
	// unsigned long long receive_video_index = 0;
	unsigned int receive_frame_count = 0;
	unsigned long receive_video_index = 0;
	unsigned long prev_video_index = 0;
	bool receive_frame_start = false;
	bool frist_receive_i_frame = false;
	bool frist_tuya_i_frame = false;
	int send_tuya_frame_count = 0;

	record_data_node node;

	char *buf_ptr = NULL;
	char *buffer = ak_mem_alloc(MODULE_ID_VDEC, VIDEO_PACKAGE_SIZE_MAX);
	// char receive_frame_key_frame = 0;
	if (get_p2p_online_status())
	{
		ipc_app_sync_utc_time();
	}

	node.data = ak_mem_alloc(MODULE_ID_VDEC, VIDEO_FRAME_MAX);
	node.is_video = true;
	// extern unsigned long long os_get_ms();
	// unsigned long long x = os_get_ms();
	printf("network_video_receive_eth_id =================>>>%d\n\r", network_video_receive_eth_id);
	while (networK_video_receive_task_run == true || ss_event_status != SS_FINISH)
	{
		FD_ZERO(&readfds);
		/*将所要检测端socket句柄加入到集合中*/
		FD_SET(video_package_receive_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 100 * 1000; // 5000;
		/*设置select等待的最大时间 检测集合read中的句柄是否有可读信息*/
		int ret_select = select(video_package_receive_fd + 1, &readfds, NULL, NULL, &timeout);

		int tuya_record_status = tuya_ipc_ss_get_status();
// printf("-----------------------%d:%d \n",ss_start_event,monitor_enter_way_get());
#if 1
		if (ss_event_status == SS_IDLE && ((monitor_enter_way_get() == MONITOR_ENTER_MONTION) || monitor_enter_way_get() == MONITOR_ENTER_CALL) && (is_sdcard_insert() == true) && (tuya_record_status != E_STORAGE_ONGOING) && (tuya_record_status != E_STORAGE_START) && (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED))
		{
			bool get_tuya_audio_ring_buffer_append_status(void);
			if (get_tuya_audio_ring_buffer_append_status() && send_tuya_frame_count > 5)
			{
				ss_event_status = SS_ONGING;

				// tuya_ipc_ring_buffer_video_release_data();
				tuya_ipc_ss_start_event();
				Debug_Lib("tuya_ipc_ss_start_event==================================================>>>>>:%d ,%lld ms \n", tuya_ipc_ss_get_status(), os_get_ms());
			}
		}
		else if ((networK_video_receive_task_run == false || tuya_event_state_get() == TRANS_LIVE_VIDEO_START || (is_sdcard_insert() == false) || tuya_monitor_state_get()) && ss_event_status == SS_ONGING)
		{
			ss_event_status = SS_STOP;
			if ((tuya_record_status != E_STORAGE_READY_TO_STOP) && (tuya_record_status != E_STORAGE_STOP))
			{
				extern void tuya_stream_storage_stop(bool keep_upload);
				/* 当前设备或其他设备没有进入涂鸦监控或者其他设备，将上传黑屏数据结束回访，反之则不上传 */
				tuya_stream_storage_stop(tuya_event_state_get() != TRANS_LIVE_VIDEO_START && !tuya_monitor_state_get());
			}
			// tuya_ipc_ss_stop_event();
			Debug_Lib("tuya_ipc_ss_stop_event==================================================>>>>> ,%lld ms\n", os_get_ms());
		}
		else if (1)
		{
			if (networK_video_receive_task_run == false)
			{
				if (ss_event_status == SS_ONGING)
				{
				}
				else
				{
					ss_event_status = SS_FINISH;
					break;
				}
#if 0
				Debug_Lib("TUYA_IPC_SS_GET_STATUS:%d,ss_event_status:%d \n",tuya_record_status,ss_event_status);
				static long int i = 0;
				extern void tuya_blank_screen_upload(MEDIA_FRAME_TYPE_E);
				i ++;
				MEDIA_FRAME_TYPE_E type = (i %5) == 0 ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME;
				tuya_blank_screen_upload(type);

				ak_sleep_ms(50);
				continue;
#endif
			}
		}
#endif

		if (ret_select > 0)
		{
			/*如果这个被监视端句柄真的变为可读了*/
			if (FD_ISSET(video_package_receive_fd, &readfds))
			{
				// int ret = recv(video_package_receive_fd, buffer, sizeof(buffer), 0);
				int ret = recvfrom(video_package_receive_fd, buffer, VIDEO_PACKAGE_SIZE_MAX, 0, NULL, NULL);

				if (ret > 0)
				{
					buf_ptr = &buffer[0];
					// ret -= 60;
					while (ret > 0)
					{
						if (memcmp(buf_ptr, video_start_code, 4) == 0)
						{
							receive_frame_count = 0;

							node.len = (buf_ptr[4] << 24) | (buf_ptr[5] << 16) | (buf_ptr[6] << 8) | buf_ptr[7];
							node.pts = (buf_ptr[8] << 24) | (buf_ptr[9] << 16) | (buf_ptr[10] << 8) | buf_ptr[11];
							receive_video_index = (buf_ptr[12] << 24) | (buf_ptr[13] << 16) | (buf_ptr[14] << 8) | buf_ptr[15];
							node.type = buf_ptr[16];
							buf_ptr += 17;

							// receive_frame_key_frame  =buf_ptr[16]?true:false;

							// printf("receive_video_index==========================>>> index : %ld \n\r", receive_video_index);

							if (frist_receive_i_frame == false)
							{
								frist_receive_i_frame = h264_is_keyframe((const unsigned char *)(buf_ptr + 4), 0) ? true : false;
								prev_video_index = receive_video_index;
							}
							// else if(receive_video_index == prev_video_index)
							// {
							// 	continue;
							// }
							else if (receive_video_index == (prev_video_index + 1))
							{
								prev_video_index = receive_video_index;
							}
							else
							{
								frist_receive_i_frame = h264_is_keyframe((const unsigned char *)(buf_ptr + 4), 0) ? true : false;
								printf("skip   ==========================>>> index : %ld \n\r", receive_video_index - 1);
							}

							ret -= 17;
							if ((ret <= 0) || (node.len > VIDEO_FRAME_MAX) || (frist_receive_i_frame == false)) /* */
							{
								receive_frame_start = false;
							}
							else
							{
								receive_frame_start = true;
							}
						}
						// printf("receive   ==========================>>> index : %ld \n\r", receive_video_index );
						// fflush(stdout);
						if (receive_frame_start == true)
						{
							// printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA receive : 0x%x \n\r",buf_ptr[4]);
							if ((receive_frame_count + ret) <= node.len)
							{
								memcpy(&node.data[receive_frame_count], buf_ptr, ret);
								receive_frame_count += ret;
								ret = 0;
								if (receive_frame_count == node.len)
								{
									receive_frame_start = false;

									int frame_type = h264_is_keyframe((const unsigned char *)(node.data + 4), node.len - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME;
									// static unsigned long long x;
									// if(frame_type == E_VIDEO_I_FRAME)
									// Debug_Lib("receive_I_frame_size:%d,0x%x \n",receive_frame_count,*(node.data + 5));
									// 	x = os_get_ms() ;
									if (/* tuya_record_status != E_STORAGE_STOP ||  */ (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED && (tuya_event_state_get() == TRANS_LIVE_VIDEO_START || monitor_enter_way_get() == MONITOR_ENTER_MONTION || monitor_enter_way_get() == MONITOR_ENTER_CALL)))
									{
										if (!tuya_monitor_state_get() && networK_video_receive_task_run)
										{
											if (frist_tuya_i_frame == false && frame_type == E_VIDEO_I_FRAME)
											{
												if (tuya_event_state_get() == TRANS_LIVE_VIDEO_START)
												{
													/* 视频通道为子通道 */
													// if(node.ch == 1)
													{
														frist_tuya_i_frame = true; /* 涂鸦上传第一帧需要是I帧 */
														Debug_Lib("tuya_ipc_ring_buffer_append_data\n");
													}
												}
												else
												{
													frist_tuya_i_frame = true; /* 涂鸦上传第一帧需要是I帧 */
													Debug_Lib("tuya_ipc_ring_buffer_append_data\n");
												}
												void tuya_upload_disable(void);
												tuya_upload_disable();
											}

#if 1
											if (frist_tuya_i_frame)
											{
												send_tuya_frame_count++;
												tuya_ipc_ring_buffer_append_data(E_CHANNEL_VIDEO_MAIN,
																				 node.data,
																				 node.len,
																				 frame_type,
																				 os_get_ms());
											}
#else

											extern void tuya_blank_screen_upload(MEDIA_FRAME_TYPE_E type);
											tuya_blank_screen_upload(h264_is_keyframe((const unsigned char *)(node.data + 4), node.len - 4) ? E_VIDEO_I_FRAME : E_VIDEO_PB_FRAME);
#endif
										}
										else
										{
											// Debug_Lib(":%d ,%lld ms\n",tuya_ipc_ss_get_status(),os_get_ms());
											extern void tuya_occupted_upload(void);
											tuya_occupted_upload();
										}
									}

									if ((tuya_online_clinet_num_get() <= 0) && (tuya_event_state_get() != TRANS_LIVE_VIDEO_START) && networK_video_receive_task_run)
									{
// printf("==================================================>>>>>:%d :%d\n",node.len - 4,node.is_video);
#ifdef LINK_LIST_ENABLE
										extern void video_vdec_push(unsigned char *data, int len);
										video_vdec_push(node.data, node.len);
#else
										video_decode_push(0, node.data, node.len);
#endif

										video_record_data_push(&node);
									}

									ak_sleep_ms(1);
								}
							}
							else
							{
								frist_receive_i_frame = false;
								printf("video unknow data:ret = %d count :%d\n", ret, receive_frame_count);
								fflush(stdout);
								ret = 0;
							}
						}
						else
						{
							frist_receive_i_frame = false;
							printf("video unknow data:receive_video_index = %ld\n", receive_video_index);
							fflush(stdout);
							ret = -1;
						}
					}
				}
			}
		}
		else
		{
			ak_sleep_ms(1);
		}
	}

	ak_mem_free(buffer);
	ak_mem_free(node.data);

	network_video_receive_socket_close();
	network_video_receive_thread_run = false;

	Debug_Lib("tuya_ipc_ss_get_status==================================================>>>>>:%d \n", tuya_ipc_ss_get_status());
	printf("===========<<< network video receive finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

bool network_video_receive_wait_thread_status(void)
{
	return network_video_receive_thread_run;
}
static bool network_video_receive_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_video_receive_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

ak_pthread_t receive_thread_id;
bool network_video_receive_package_open(unsigned long id)
{
	if (networK_video_receive_task_run == true)
	{
		return false;
	}

	if (network_video_receive_wait_thread_quit() == false)
	{
		return false;
	}

	static int eth_p_id;
	eth_p_id = id;
	networK_video_receive_task_run = true;
	printf("%s=============>>>0x%x\n", __func__, eth_p_id);

	ak_thread_create(&receive_thread_id, network_video_receive_package_task, &eth_p_id, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(receive_thread_id);
	return true;
}

int curr_network_video_receive_eth_id_get(void)
{
	return network_video_receive_eth_id;
}

bool network_video_receive_package_close(void)
{
	if (networK_video_receive_task_run == false)
	{
		return false;
	}
	receive_frame_push = false;
	networK_video_receive_task_run = false;
	network_video_receive_eth_id = 0;
	video_decode_queue_reset();
	// ak_thread_join(receive_thread_id);

	return true;
}
