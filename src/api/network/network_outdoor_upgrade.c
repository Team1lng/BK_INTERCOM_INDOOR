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
//#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "ak_thread.h"
#include "ak_mem.h"
#include <net/if_arp.h>
#include <netinet/in.h>
//#include<netinet/ip.h>
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

#define FILE_PATH "cbin.update"
#define UPGRADE_FILE_PATH "/mnt/tf/"FILE_PATH
#define UPGRADE_PACKAGE_SIZE_MAX 1510 



static bool network_upgrade_sent_task_run = false;//任务

static ak_mutex_t network_upgrade_send_mutex;
#if 0

static int network_upgrade_send_eth_id = 0;
static int upgrade_package_send_fd = -1;
static const char upgrade_start_code[4] = {0x00, 0x00, 0x01, 0xfc};



static bool network_upgrade_send_socket_open(void)
{
	if (upgrade_package_send_fd != -1)
	{
		return false;
	}

	printf("==========>>> upgrade send socket %0x <<<==========\n", network_upgrade_send_eth_id);
	if ((upgrade_package_send_fd = socket(PF_PACKET, SOCK_RAW, htons(network_upgrade_send_eth_id))) < 0)
	{
		printf("create socket error raw_socket_receive_fd\n");
		return false;
	}

	return true;
}

static bool network_upgrade_send_socket_close(void)
{
	if (upgrade_package_send_fd == -1)
	{
		return false;
	}

	close(upgrade_package_send_fd);
	upgrade_package_send_fd = -1;

	return true;
}


static void network_send_package_upgrade(network_upgrade_data *node){
	network_upgrade_data *package = node;
	int read_size = package->len;
	int send_size = 0;
	char *buffer = (char *)ak_mem_alloc(MODULE_ID_APP, UPGRADE_PACKAGE_SIZE_MAX);
		memset(buffer, 0, UPGRADE_PACKAGE_SIZE_MAX);
	while(read_size > 0){
		
		memcpy(&buffer[0], nework_get_package_head(network_upgrade_send_eth_id), 60);
		if (send_size == 0)
		{
			memcpy(&buffer[60], upgrade_start_code, 4);
			buffer[64] = (package->index >> 24) & 0xFF;//第几个包
			buffer[65] = (package->index >> 16) & 0xFF;
			buffer[66] = (package->index >> 8) & 0xFF;
			buffer[67] = package->index & 0xFF;

			buffer[68] = (package->len >> 24) & 0xFF;//长度
			buffer[69] = (package->len >> 16) & 0xFF;
			buffer[70] = (package->len >> 8) & 0xFF;
			buffer[71] = package->len & 0xFF;

			if (read_size > (UPGRADE_PACKAGE_SIZE_MAX - 72))
			{
				memcpy(&buffer[72], &package->buf[send_size], UPGRADE_PACKAGE_SIZE_MAX - 72);
				//write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
				if (sendto(upgrade_package_send_fd, buffer, UPGRADE_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
				{
					perror(" send to fail \n");
				}
				read_size -= (UPGRADE_PACKAGE_SIZE_MAX - 72);
				send_size += (UPGRADE_PACKAGE_SIZE_MAX - 72);
			}else
			{
				memcpy(&buffer[72], &package->buf[send_size], read_size);
				//write(file_fd,buffer,remain_size + 31);
				if (sendto(upgrade_package_send_fd, buffer, read_size + 72, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
				{
					perror(" send to fail \n");
				}
				//	printf("send audio size:%d \n",remain_size);
				break;
			}

		}
		if (read_size > (UPGRADE_PACKAGE_SIZE_MAX - 60))
		{
			memcpy(&buffer[60], &package->buf[send_size], UPGRADE_PACKAGE_SIZE_MAX - 60);
			//	write(file_fd,buffer,VIDEO_PACKAGE_SIZE_MAX);
			if (sendto(upgrade_package_send_fd, buffer, UPGRADE_PACKAGE_SIZE_MAX, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			read_size -= (UPGRADE_PACKAGE_SIZE_MAX - 60);
			send_size += (UPGRADE_PACKAGE_SIZE_MAX - 60);
		}
		else
		{
			memcpy(&buffer[60], &package->buf[send_size], read_size);
			//write(file_fd,buffer,remain_size + 14);
			if (sendto(upgrade_package_send_fd, buffer, read_size + 60, 0, (struct sockaddr *)network_get_send_addres(), sizeof(struct sockaddr_ll)) < 0)
			{
				perror(" send to fail \n");
			}
			break;
		}

	}	
	ak_mem_free(buffer);

}
#endif

static int file_size_get(FILE* fp){
    fseek(fp,0,SEEK_END);
    int file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
    return file_size;
}

static void printf_progress(int percentage) {

    // char buff[128] = {0};
    // int str_len = 0;
    printf("%d%%", percentage);
    putc('\b', stdout);
    putc('\b', stdout);
    putc('\b', stdout);
    fflush(stdout);
}


extern bool cancle_falg;
static void *network_upgrade_send_package_task(void *arg){

	

	ak_thread_mutex_init(&network_upgrade_send_mutex, NULL);

	system("cp /mnt/tf/"FILE_PATH" /tmp");
	FILE *fp = fopen("/tmp/"FILE_PATH"", "r");//只读打开文件
	if(fp == NULL) //打开失败
	{
		printf("=====>>>>> upgrade file open err <<<<<=====\n");
		return NULL;//返回 直接结束
	}

	
	int file_size = file_size_get(fp);//获取到文件总大小
	int idnex_num = 0;//发包的数量
	int sent_size = 0;//发送的文件长度
	network_upgradecmd_data node ;//包
	memset(&node, 0, sizeof(network_upgradecmd_data));


	//创建进度条，开始升级   

	// extern bool upgrade_event_push(char ,char );
	// upgrade_event_push(1,0);
	/*
	lv_obj_t* obj = lv_obj_get_child_form_id(lv_scr_act(), 66);
	lv_obj_t* bar = NULL;
	if(obj != NULL){
		
		bar = lv_bar_create(lv_obj_get_child(obj, NULL), NULL);
	
		lv_obj_set_size(bar, 354, 10);
		lv_obj_align(bar, bar->parent, LV_ALIGN_IN_TOP_MID, 0, 130);
		lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x393939));
		lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
		lv_bar_set_range(bar, 0, 100);
		lv_bar_set_start_value(bar, 0, LV_ANIM_OFF);

	}
	
	*/
	
	
	while(network_upgrade_sent_task_run == true){
		
		int ret = 0;
		//ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁
		extern int upgrade_pack_len_get(void);
		if((ret = fread(node.buf,sizeof(char), upgrade_pack_len_get(),fp)) > 0)//读取到数据
		{
			//ak_thread_mutex_unlock(&network_upgrade_send_mutex);
			node.cmd = NET_COMON_CMD_UPGRADE_OUTDOOR;
			
			node.device = DEVICE_ALL;
			node.arg1 = ++idnex_num;//发送的第几个包
			node.arg2 = ret;//数据长度
		
			sent_size += ret;//已经发送的数据长度
			printf("====>>>> read file num:%d  len:%d <<<<=====\n",idnex_num,ret);
			
			network_sendupgrade_cmd_data(&node);//封包和发送
			
			
			printf_progress(sent_size*100/file_size);



			//升级进度显示          
			extern bool upgrade_event_push(char ,char );
			upgrade_event_push(4,sent_size*100/file_size);
			//lv_bar_set_value(bar, sent_size*100/file_size, LV_ANIM_OFF);
			
			ak_sleep_ms(50);
			
		}
		else if(ret == 0){
			//ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁ak_thread_mutex_lock(&network_upgrade_send_mutex);	//上锁
			/*arg1 == 1检查设备是否在线 arg1 == 2 arg2 == 1结束发送*/
			network_cmd_data data;

			data.cmd			= NET_COMON_CMD_UPGRADE_OUTDOOR;
			data.arg1			= 2;
			data.arg2			= 1;
			data.device 		= DEVICE_ALL;
			network_send_cmd_data(&data);
			printf("read file end! file_len =:%d sent_len =:%d\n",file_size,sent_size);

			//升级完成，删除进度条 
			extern bool upgrade_event_push(char ,char );
			upgrade_event_push(5,0);
			//lv_obj_del(bar);
			
			network_upgrade_sent_task_run = false;
		}

		//ak_thread_mutex_unlock(&network_upgrade_send_mutex);
		
		
	}
		
	fclose(fp);
	system("rm /tmp/"FILE_PATH"");
	network_upgrade_sent_task_run = false;

	ak_thread_exit();
	return NULL;

}

static int upgrade_pack_len = UPGRADE_PACK_LEN;

void upgrade_pack_len_set(int len)
{
	upgrade_pack_len = len;
}
int upgrade_pack_len_get(void)
{
	return upgrade_pack_len;
}

static ak_pthread_t thread_upgrade_id;

bool network_upgrade_send_package_open(void){
	if(network_upgrade_sent_task_run == true){
		return false;
	}
	
	network_upgrade_sent_task_run = true;
	
	ak_thread_create(&thread_upgrade_id, network_upgrade_send_package_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_upgrade_id);

	return true;
}




//发送任务关闭
bool network_upgrade_sent_package_close(void){
	if(network_upgrade_sent_task_run == false){
		
		return false;
	}else{
		network_upgrade_sent_task_run = false;
		ak_thread_cancel(thread_upgrade_id);

	}
	
	
	return true;
}





