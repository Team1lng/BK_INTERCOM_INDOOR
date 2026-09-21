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
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "ak_thread.h"
#include "ak_mem.h"
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
#include "tuya_ipc_media.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_p2p.h"
#include "../../include/g711/g711_table.h"
#include "ak_mem.h"
#include "tuya_g711_utils.h"
#include "tuya_sdk.h"
#include "ak_common_audio.h"

#define AUDIO_QUEUE_PACKAGE_MAX 128 // 5

#define AUDIO_PACKAGE_SIZE_MAX 1510 // 10*1024//1510

#define AUDIO_FRAME_MAX (2 * 1024)
static int network_audio_send_eth_id = 0;

static const char audio_start_code[4] = {0x00, 0x00, 0x01, 0xfc};

struct sockaddr_ll *network_get_send_addres(void);
const char *nework_get_package_head(unsigned int type);
bool video_record_data_push(record_data_node *node);
bool audio_decode_queue_push(unsigned char *data, int len);

typedef struct
{
	char frame_type; // 0:pcm
	char *data;
	int len;
	unsigned long long pts;
	unsigned long long frame_index;
} network_audio_data;

typedef struct
{
	void *prev;
	void *next;

	bool ring_back;
	network_audio_data package;
} network_audio_package;
static queue_s network_audio_send_queue_free;
static queue_s network_audio_send_queue_head;

static ak_mutex_t network_audio_send_head_mutex;
static ak_mutex_t network_audio_send_free_mutex;

//  static int send_audio_fd = -1;

static network_audio_package network_audio_package_buffer[AUDIO_QUEUE_PACKAGE_MAX];
static void network_audio_send_package_queue_init(void)
{
	static bool is_first = true;
	if (is_first == false)
	{
		return;
	}
	is_first = false;

	ak_thread_mutex_init(&network_audio_send_head_mutex, NULL);
	ak_thread_mutex_init(&network_audio_send_free_mutex, NULL);

	queue_initialize(&network_audio_send_queue_head);
	queue_initialize(&network_audio_send_queue_free);
	memset(network_audio_package_buffer, 0, sizeof(network_audio_package) * AUDIO_QUEUE_PACKAGE_MAX);
	for (int i = 0; i < AUDIO_QUEUE_PACKAGE_MAX; i++)
	{
		queue_insert((queue_s *)&network_audio_package_buffer[i], &network_audio_send_queue_free);
	}
}

static network_audio_package *network_audio_send_package_queue_new(char type, const char *data, int size)
{
	network_audio_package *node = NULL;
	ak_thread_mutex_lock(&network_audio_send_free_mutex);
	if (queue_empty(&network_audio_send_queue_free) == 0)
	{
		node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_free);
	}
	ak_thread_mutex_unlock(&network_audio_send_free_mutex);
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
	node->package.len = size;
	node->package.pts = get_sys_ms();
	node->package.frame_type = 0;
	// printf("send index :%llu \n",node->package.frame_index);
	return node;
}
static void network_audio_send_package_queue_del(network_audio_package *node)
{
	if (node != NULL)
	{
		if (node->package.data != NULL)
		{
			ak_mem_free(node->package.data);
			node->package.data = NULL;
		}

		ak_thread_mutex_lock(&network_audio_send_free_mutex);
		queue_insert((queue_s *)node, &network_audio_send_queue_free);
		ak_thread_mutex_unlock(&network_audio_send_free_mutex);
	}
}

static void network_audio_send_release_all(void)
{
	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	while (!queue_empty(&network_audio_send_queue_head))
	{
		network_audio_package *node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_head);
		network_audio_send_package_queue_del(node);
	}
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);
}

static bool tuya_audio_ring_buffer_append = false;

bool get_tuya_audio_ring_buffer_append_status(void)
{
	return tuya_audio_ring_buffer_append;
}

static bool network_audio_send_thread_run = false;
static bool networK_audio_send_task_run = false;
static bool network_audio_send_ready = false;
static int audio_package_send_fd = -1;
static bool network_audio_send_socket_open(void)
{
	if (audio_package_send_fd != -1)
	{
		return false;
	}

	printf("==========>>> audio send socket %0x <<<==========\n", network_audio_send_eth_id);
	if ((audio_package_send_fd = socket(PF_PACKET, SOCK_RAW, htons(network_audio_send_eth_id))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

	return true;
}

static bool network_audio_send_socket_close(void)
{
	if (audio_package_send_fd == -1)
	{
		return false;
	}

	close(audio_package_send_fd);
	audio_package_send_fd = -1;

	return true;
}

static void network_send_package_audio(network_audio_package *node)
{
	// printf("%s==============\n",__func__);
	network_audio_data *package = &(node->package);
	int send_size = 0, remain_size = package->len;
	// printf("%s============== package->len:%d\n",__func__,package->len);

	char *buffer = (char *)ak_mem_alloc(MODULE_ID_APP, AUDIO_PACKAGE_SIZE_MAX);
	memset(buffer, 0, AUDIO_PACKAGE_SIZE_MAX);

	// int file_fd = open("/mnt/write.h264",O_CREAT|O_WRONLY);
	while (remain_size > 0)
	{
		memcpy(&buffer[0], nework_get_package_head(network_audio_send_eth_id), 60);
		if (send_size == 0)
		{
			memcpy(&buffer[60], audio_start_code, 4);
			buffer[64] = (remain_size >> 24) & 0xFF;
			buffer[65] = (remain_size >> 16) & 0xFF;
			buffer[66] = (remain_size >> 8) & 0xFF;
			buffer[67] = remain_size & 0xFF;

			buffer[68] = (package->pts >> 24) & 0xFF;
			buffer[69] = (package->pts >> 16) & 0xFF;
			buffer[70] = (package->pts >> 8) & 0xFF;
			buffer[71] = package->pts & 0xFF;

			buffer[72] = (package->frame_index >> 24) & 0xFF;
			buffer[73] = (package->frame_index >> 16) & 0xFF;
			buffer[74] = (package->frame_index >> 8) & 0xFF;
			buffer[75] = package->frame_index & 0xFF;
			buffer[76] = package->frame_type;

			if (remain_size > (AUDIO_PACKAGE_SIZE_MAX - 77))
			{
				memcpy(&buffer[77], &package->data[send_size], AUDIO_PACKAGE_SIZE_MAX - 77);
				// write(send_audio_fd,&package->data[send_size],AUDIO_PACKAGE_SIZE_MAX - 77);
				if (sendto(audio_package_send_fd, buffer, AUDIO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
				{
					perror(" send to fail \n");
				}
				remain_size -= (AUDIO_PACKAGE_SIZE_MAX - 77);
				send_size += (AUDIO_PACKAGE_SIZE_MAX - 77);
			}
			else
			{
				memcpy(&buffer[77], &package->data[send_size], remain_size);
				// write(send_audio_fd,&package->data[send_size],remain_size);
				if (sendto(audio_package_send_fd, buffer, remain_size + 77, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
				{
					perror(" send to fail \n");
				}
				// printf("send audio size:%d \n",remain_size);
				break;
			}
		}

		// ak_sleep_ms(1);
		if (remain_size > (AUDIO_PACKAGE_SIZE_MAX - 60))
		{
			memcpy(&buffer[60], &package->data[send_size], AUDIO_PACKAGE_SIZE_MAX - 60);
			// write(send_audio_fd,&package->data[send_size],AUDIO_PACKAGE_SIZE_MAX - 60);
			if (sendto(audio_package_send_fd, buffer, AUDIO_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			remain_size -= (AUDIO_PACKAGE_SIZE_MAX - 60);
			send_size += (AUDIO_PACKAGE_SIZE_MAX - 60);
		}
		else
		{
			memcpy(&buffer[60], &package->data[send_size], remain_size);
			// write(file_fd,buffer,remain_size + 14);
			// write(send_audio_fd,&package->data[send_size],remain_size);
			if (sendto(audio_package_send_fd, buffer, remain_size + 60, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			break;
		}
	}
	ak_mem_free(buffer);
	// close(file_fd);
	// printf("write finish \n");
	// while(1);
}

bool pcm_resamplerate(int src_sample, int dst_sample, const char *src_pcm, int src_frames, unsigned char *dst_pcm, int *dst_frames);
static void *network_audio_send_package_task(void *arg)
{
	network_audio_send_thread_run = true;

	pcm16_alaw_tableinit();

	network_audio_send_socket_open();

	network_audio_send_ready = true;

#ifdef RING_BACK_MODIFY
	unsigned char cover_data[4096] = {0};
	int main_len = 0;
	network_audio_package ring_back_node;
	unsigned char temp[4096];
	int cover_len = 0;
#endif

	while (networK_audio_send_task_run == true)
	{
		// unsigned long long os_get_ms(void);
		// static unsigned long long frist_count_ms = 0;
		// static int count = 0;
		// count ++;
		// if(count  == 100)
		// {
		// 	count = 0;
		// 	// printf("push end ms =================>>>%lld\n\r",os_get_ms());
		// 	printf("network_audio_send_package_task  ms =================>>>%lld\n\r",(os_get_ms() - frist_count_ms)/100);
		// 		frist_count_ms = os_get_ms();
		// }

		network_audio_package *node = NULL;
		ak_thread_mutex_lock(&network_audio_send_head_mutex);
		if (queue_empty(&network_audio_send_queue_head) == 0)
		{
			node = (network_audio_package *)queue_delete_next(&network_audio_send_queue_head);
		}
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);

		if (node != NULL)
		{
#ifdef RING_BACK_MODIFY
			if (node->ring_back)
			{

				// printf("==========>>>network_audio_send_package_task %0d <<<==========\n", node->ring_back);
				memset(temp, 0, sizeof(temp));
				cover_len = 0;

				extern enum ak_audio_sample_rate get_curr_audio_sample_rate(void);
				pcm_resamplerate(get_curr_audio_sample_rate(), 16000, (const char *)node->package.data, node->package.len, temp, &cover_len);

				if ((main_len + cover_len) > 4096)
				{
					int alaw_buffer_size = main_len / 2;
					ring_back_node.package.data = ak_mem_alloc(MODULE_ID_APP, alaw_buffer_size);
					if (ring_back_node.package.data == NULL)
					{
						printf("Error while allocating memory for node.data.\n");
					}

					pcm16_to_alaw(main_len, (const char *)cover_data, ring_back_node.package.data);

					ring_back_node.package.len = alaw_buffer_size;
					ring_back_node.package.frame_type = 1;
					network_send_package_audio(&ring_back_node);
					ak_mem_free(ring_back_node.package.data);
					main_len = 0;
				}
				memcpy(&cover_data[main_len], temp, cover_len);
				main_len += cover_len;
			}
			else
#endif
			{
				network_send_package_audio(node);
			}

			network_audio_send_package_queue_del(node);
		}
		else
		{
			ak_sleep_ms(1);
		}
	}

	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	network_audio_send_ready = false;
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);

	network_audio_send_socket_close();
	network_audio_send_release_all();

	network_audio_send_thread_run = false;

	// close(send_audio_fd);
	// send_audio_fd = -1;
	// system("sync");
	network_audio_send_eth_id = -1;
	printf("===========<<< network audio send finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_audio_send_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_audio_send_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

int network_audio_send_task_eth_id(void)
{
	return network_audio_send_eth_id;
}

bool network_audio_send_package_open(unsigned long id)
{
	network_audio_send_package_queue_init();

	if (networK_audio_send_task_run == true)
	{
		return false;
	}

	if (network_audio_send_wait_thread_quit() == false)
	{
		return false;
	}

	network_audio_send_eth_id = id;
	networK_audio_send_task_run = true;

	// send_audio_fd = open("/tmp/send_audio.pcm",O_WRONLY|O_CREAT);
	// if(send_audio_fd < 0)
	// {
	// 	printf("write open tmp/send_audio.pcm fail\n");
	// }
	// else{
	// 	printf("write open tmp/send_audio.pcm O_WRONLY|O_CREAT succeed :%d\n",send_audio_fd);
	// }

	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_audio_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

bool network_audio_send_package_close(void)
{
	if (networK_audio_send_task_run == false)
	{
		return false;
	}
	networK_audio_send_task_run = false;
	return true;
}

bool network_audio_send_package_push(char type, const char *data, int len, bool ring_back)
{
	ak_thread_mutex_lock(&network_audio_send_head_mutex);
	if (network_audio_send_ready == false)
	{
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);
		return false;
	}

	network_audio_package *node = network_audio_send_package_queue_new(type, data, len);
	if (node == NULL)
	{
		printf("network_audio_send_package_push full\n\r");
		ak_thread_mutex_unlock(&network_audio_send_head_mutex);
		return false;
	}
	node->ring_back = ring_back;
	queue_insert((queue_s *)node, &network_audio_send_queue_head);
	ak_thread_mutex_unlock(&network_audio_send_head_mutex);
	return true;
}

static int network_audio_receive_eth_id = 0;
static bool network_audio_receive_thread_run = false;
static bool networK_audio_receive_task_run = false;
static int audio_package_receive_fd = -1;
static bool network_audio_receive_socket_open(void)
{
	if (audio_package_receive_fd != -1)
	{
		return false;
	}
	printf("==========>>> audios receive socket %0x <<<==========\n", network_audio_receive_eth_id);
	if ((audio_package_receive_fd = socket(PF_PACKET, SOCK_RAW, htons(network_audio_receive_eth_id))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

	struct ifreq req;
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	strcpy(req.ifr_name, NETWORK_NAME);
	ioctl(fd, SIOCGIFINDEX, &req);
	close(fd);

	struct sockaddr_ll sll;
	struct packet_mreq mr;
	memset(&sll, 0, sizeof(sll));
	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = req.ifr_ifindex;
	sll.sll_protocol = htons(network_audio_receive_eth_id);
	if (bind(audio_package_receive_fd, (struct sockaddr *)&sll, sizeof(sll)) == -1)
	{
		perror("bind");
		return (1);
	}

	memset(&mr, 0, sizeof(mr));
	mr.mr_ifindex = req.ifr_ifindex;
	mr.mr_type = PACKET_MR_PROMISC;
	if (setsockopt(audio_package_receive_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
	{
		perror("setsockopt");
		return (1);
	}

	int recv_buf_size = 10 * 1024;
	if (setsockopt(audio_package_receive_fd, SOL_SOCKET, SO_RCVBUF, &recv_buf_size, sizeof(int)) < 0)
	{
		printf("setsockopt SO_RCVBUF\n");
		return false;
	}
	return true;
}

static bool network_audio_receive_socket_close(void)
{
	if (audio_package_receive_fd == -1)
	{
		return false;
	}

	close(audio_package_receive_fd);
	audio_package_receive_fd = -1;

	return true;
}

unsigned long long os_get_ms(void)
{
	struct ak_timeval tv;
	ak_get_ostime(&tv);
	return tv.usec / 1000 + tv.sec * 1000;
}

static bool audio_data_push_decode = false;
void audio_data_decode_power(bool enable)
{
	audio_data_push_decode = enable;
}

static void *network_audio_receive_package_task(void *arg)
{
	fd_set readfds;
	struct timeval timeout;

	network_audio_receive_thread_run = true;

	alaw_pcm16_tableinit();
	network_audio_receive_socket_open();

#define PCM_BUFFER_MAX 1048 * 2
	record_data_node node;
	node.is_video = false;
	node.data = ak_mem_alloc(MODULE_ID_APP, PCM_BUFFER_MAX);
	node.len = 0;

	char *receive_frame_buffer = ak_mem_alloc(MODULE_ID_APP, AUDIO_FRAME_MAX);
	// unsigned long long receive_frame_pts = 0;
	// unsigned long long receive_video_index = 0;
	unsigned int receive_frame_size = 0;
	unsigned int receive_frame_count = 0;
	bool receive_frame_start = false;

	char *buf_ptr = NULL;
	char buffer[AUDIO_PACKAGE_SIZE_MAX] = {0};

	tuya_audio_ring_buffer_append = false;

	// int fb = open("/mnt/nfs/write.pcm",O_CREAT|O_WRONLY);

	printf("===========<<< network audio receive start >>>===========\n");
	while (networK_audio_receive_task_run == true)
	{
		FD_ZERO(&readfds);
		/*将所要检测端socket句柄加入到集合中*/
		FD_SET(audio_package_receive_fd, &readfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 100 * 1000;
		/*设置select等待的最大时间 检测集合read中的句柄是否有可读信息*/
		int ret_select = select(audio_package_receive_fd + 1, &readfds, NULL, NULL, &timeout);
		if (ret_select > 0)
		{
			/*如果这个被监视端句柄真的变为可读了*/
			if (FD_ISSET(audio_package_receive_fd, &readfds))
			{
				int ret = recvfrom(audio_package_receive_fd, buffer, sizeof(buffer), 0, NULL, NULL);
				// Debug_Lib("ret:%d\n", ret);
				if (ret > 60)
				{
					// write(file_fd,buffer,ret);
					buf_ptr = &buffer[60];
					ret -= 60;
					while (ret > 0)
					{
						if (memcmp(buf_ptr, audio_start_code, 4) == 0)
						{
							receive_frame_count = 0;

							receive_frame_size = (buf_ptr[4] << 24) | (buf_ptr[5] << 16) | (buf_ptr[6] << 8) | buf_ptr[7];
							node.pts = (buf_ptr[8] << 24) | (buf_ptr[9] << 16) | (buf_ptr[10] << 8) | buf_ptr[11];
							// receive_video_index =  (buf_ptr[12] << 24) | (buf_ptr[13] << 16) | (buf_ptr[14] << 8) | buf_ptr[15];
							if (buf_ptr[16])
							{
								ret = -1;
								continue;
							}

							//	receive_frame_key_frame  =buf_ptr[16]?true:false;
							buf_ptr += 17;
							ret -= 17;
							if (ret < 0)
							{
								receive_frame_start = false;
							}
							else
							{
								receive_frame_start = true;
							}
						}

						if (receive_frame_start == true)
						{
							if ((receive_frame_count + ret) <= receive_frame_size)
							{
								memcpy(&receive_frame_buffer[receive_frame_count], buf_ptr, ret);
								receive_frame_count += ret;
								ret = 0;
								if (receive_frame_count == receive_frame_size)
								{
									receive_frame_start = false;
									extern TRANSFER_EVENT_E tuya_event_state_get(void);
#if 0
							
									if (tuya_online_clinet_num_get() > 0 && tuya_event_state_get() == TRANS_LIVE_VIDEO_START)
									{
										// printf("tuya audio send size : %d \n\r",node.len);
										int pcm_buffer_size = receive_frame_size *2;
										char *pcm_buffer = NULL;
										pcm_buffer = malloc(pcm_buffer_size);
										tuya_g711_decode(TUYA_G711_A_LAW,(short unsigned int *)receive_frame_buffer,receive_frame_size,(unsigned char *)pcm_buffer,(unsigned int *)&pcm_buffer_size);
										tuya_ipc_ring_buffer_append_data(9 /*E_CHANNEL_AUDIO*/, (unsigned char *)pcm_buffer, pcm_buffer_size, 3 /*E_AUDIO_FRAME*/, os_get_ms());
										free(pcm_buffer);
										pcm_buffer = NULL;
										node.len = 0;
									}
									else if (/* tuya_ipc_get_client_tuya_online_clinet_num_getonline_num() <= 0 && */ (node.len + receive_frame_size) > PCM_BUFFER_MAX)
									{

										// write(fb,pcm_buffer,pcm_buffer_size);
#if 0
										int pcm_buffer_size = node.len *2;
										char *pcm_buffer = NULL;
										pcm_buffer = malloc(pcm_buffer_size);
										if(pcm_buffer == NULL)
										{
											printf("Error while allocating memory for write buffer.\n");
											continue;
										}

										alaw_to_pcm16(node.len,(const char *)node.data,pcm_buffer);

										if (audio_data_push_decode)
										{
											audio_decode_queue_push((unsigned char *)pcm_buffer, pcm_buffer_size);
										}
										video_record_data_push(0, (unsigned char *)pcm_buffer, pcm_buffer_size, false);
									
										free(pcm_buffer);
										pcm_buffer = NULL;

#else
											if (audio_data_push_decode)
											{
												audio_decode_queue_push((unsigned char *)node.data, node.len);
											}
											video_record_data_push(0, (unsigned char *)node.data, node.len, false);

#endif


										node.len = 0;

									}

#else
									if (tuya_ipc_register_status_get() == E_IPC_ACTIVEATED && (tuya_event_state_get() == TRANS_LIVE_VIDEO_START || monitor_enter_way_get() == MONITOR_ENTER_MONTION || monitor_enter_way_get() == MONITOR_ENTER_CALL))
									{
										int pcm_buffer_size = receive_frame_size * 2;
										char *pcm_buffer = malloc(pcm_buffer_size);
										if (pcm_buffer != NULL)
										{
											tuya_g711_decode(TUYA_G711_A_LAW, (short unsigned int *)receive_frame_buffer, receive_frame_size, (unsigned char *)pcm_buffer, (unsigned int *)&pcm_buffer_size);

											if (!tuya_monitor_state_get())
											{
												// Debug_Lib(":%d ,%lld ms\n",tuya_ipc_ss_get_status(),os_get_ms());

												tuya_ipc_ring_buffer_append_data(9 /*E_CHANNEL_AUDIO*/, (unsigned char *)pcm_buffer, pcm_buffer_size, 3 /*E_AUDIO_FRAME*/, os_get_ms());
												tuya_audio_ring_buffer_append = true;
											}
											free(pcm_buffer);
										}
									}

									if ((tuya_online_clinet_num_get() <= 0 && tuya_event_state_get() != TRANS_LIVE_VIDEO_START))
									{
										if ((node.len + receive_frame_size) > PCM_BUFFER_MAX)
										{
											// Debug_Lib("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!,%d,%d,%d\n", tuya_online_clinet_num_get(), tuya_event_state_get(), node.len + receive_frame_size);
											if (audio_data_push_decode)
											{
												audio_decode_queue_push((unsigned char *)node.data, node.len);
											}
											video_record_data_push(&node);
											node.len = 0;
										}
									}
									else
									{
										node.len = 0;
									}
#endif
									memcpy(&node.data[node.len], receive_frame_buffer, receive_frame_size);
									node.len += receive_frame_size;

									// char buf[1024] = {0};
									// unsigned int len = 0;
									// tuya_g711_encode(1,receive_frame_buffer,receive_frame_size,buf,&len);

									// ak_sleep_ms(1);
								}
								else
								{
								}
							}
							else
							{
								ret = 0;
							}
						}
						else
						{
							// printf("audio unknow data:ret = %d\n", ret);
							ret = -1;
						}
					}
				}
			}
		}
		ak_sleep_ms(1);
	}

	if (receive_frame_buffer != NULL)
	{
		ak_mem_free(receive_frame_buffer);
	}
	if (node.data != NULL)
	{
		ak_mem_free(node.data);
	}
	network_audio_receive_socket_close();
	network_audio_receive_thread_run = false;
	network_audio_receive_eth_id = -1;
	// close(fb);
	printf("===========<<< network audio receive finish >>>===========\n");
	ak_thread_exit();
	return NULL;
}

static bool network_audio_receive_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (network_audio_receive_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

int network_audio_receive_task_eth_id(void)
{
	return network_audio_receive_eth_id;
}

bool network_audio_receive_package_open(unsigned long id)
{
	if (networK_audio_receive_task_run == true)
	{
		return false;
	}

	if (network_audio_receive_wait_thread_quit() == false)
	{
		return false;
	}
	network_audio_receive_eth_id = id;
	networK_audio_receive_task_run = true;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, network_audio_receive_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);

	return true;
}

bool network_audio_receive_package_close(void)
{
	if (networK_audio_receive_task_run == false)
	{
		return false;
	}
	networK_audio_receive_task_run = false;
	return true;
}
