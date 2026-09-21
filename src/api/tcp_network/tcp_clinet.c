#include"tcp_clinet.h"
#if 0
#define DEFAULT_TCP_CLINET_INITIALIZER { \
    .port = 0, \
    .ip = NULL, \
    .cmd = 0, \
    .arg1 = 0, \
    .arg2 = 0, \
    .data = NULL, \
    .data_size = 0, \
    .dev_id = 0, \
    .tid = -1 \
    .fd = -1, \
}

static tcp_clinet_t tcp_clinet_group[8] = 
{
    DEFAULT_TCP_CLINET_INITIALIZER
};

void *tcp_management_task(void*arg)
{
    tcp_network_info *info = ((tcp_network_info *)arg);

    if(info->dev_id)//避免ARP缓存中的MAC地址因门口机id切换与之前IP不相符，每次连接前清除ARP缓存
    {
        system("arp -d "OUTDOOR2_IP);
    }
    else
    {
        system("arp -d "OUTDOOR1_IP);
    }

    tcp_management_run = true;

    printf("======[ TCP_CONNECT DOOR:%d THREAD:%lu  WATI...............    !!!!! ]======\n",info->dev_id,info->thread_id);

    info->dev_fd = tcp_connect(info->dev_id ? OUTDOOR2_IP : OUTDOOR1_IP, TCP_PORT);
    if (info->dev_fd < 0)
    {
        printf("tcp_connect error thread_id :%lu!\n",info->thread_id);
        info->thread_id = 0;

        if((net_info[TCP_DEVICE_OUTDOOR_1].thread_id | net_info[TCP_DEVICE_OUTDOOR_2].thread_id) == 0)
            tcp_management_run = false;

        ak_thread_exit();
        return NULL;
    }

    if(tcp_management_run == false)//tcp_connec阻塞时手动关闭，阻塞结束后退出线程
    {
        goto exit;
    }

    printf("======[ TCP_CONNECT DOOR:%d SUCCEED   fd:0x%x !!!!! ]======\n",info->dev_id,info->dev_fd);
    tcp_network_cmd_get_data_info_send(info->dev_id);

    unsigned int next_read_len = READ_BUFFER_LEN;
    while (tcp_management_run)
    {
        char buf[READ_BUFFER_LEN] = {0};
        if (/* scanf("%s", buf) */1)
        {
            // bzero(buf, sizeof(buf));
            int recv_len = tcp_nonblocking_recv(info->dev_fd, buf, next_read_len,0,5000);
            if (recv_len == 0)
            {
                printf("tcp_blocking_recv exit!\n");
               break;
            }

            if(recv_len > 0)
            {
                printf("recv_len]======%d\n",recv_len);
            }

            if(recv_len > 0 && net_common_code_check_valid(buf,recv_len,&next_read_len,info))
            {
                net_common_event_process(info);
            }
            // printf("recv : %s\n", buf);
        } 
    }

exit:

    // card_remark_file_push(info);

    // 等待输出缓冲区变为空闲状态
    wati_tcp_process_close(info);

    tcp_close(info->dev_fd);
    info->dev_fd = -1;

    printf("======[ TCP MANAGRMENT DOOR:%d EXIT FINISH    !!!! ]======\n",info->dev_id);
    ak_thread_exit();
    return NULL;
}
#endif