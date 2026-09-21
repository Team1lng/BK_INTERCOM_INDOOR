#include "tcp_socket.h"
#include "tcp_network_cmd.h"
#include "unistd.h"
#include "ak_thread.h"
#include "ak_common.h"

#define TCP_PORT 4321
#define OUTDOOR1_IP "192.168.37.7"
#define OUTDOOR2_IP "192.168.37.8"

static tcp_network_info net_info[TCP_DEVICE_END];
static management_type manage_type = MANAGE_CARD; // 0:card 1:finger
static char data_packge_buffer[LONG_PACK_LEN] = {0};

extern bool tcp_network_event_push(unsigned long arg1, unsigned long arg2, unsigned long arg3);

bool network_short_send_common(tcp_device send_to_device, unsigned char cmd, unsigned char arg1, unsigned char arg2)
{
    tcp_network_info *info = NULL;
    if (send_to_device > TCP_DEVICE_OUTDOOR_2)
    {
        return false;
    }

    info = &net_info[send_to_device];

    if (info->dev_fd == -1)
    {
        return false;
    }
    unsigned char code[SHORT_PACK_LEN];
    code[0] = DATA_PACK_START;
    code[1] = LOCAL_DEVICE;
    code[2] = LOCAL_DEVICE;
    code[3] = cmd;
    code[4] = arg1;
    code[5] = arg2;
    code[6] = (code[1] + code[2] + code[3] + code[4] + code[5]) & 0xFF;
    code[7] = PACK_END;

    int send_len = tcp_send(info->dev_fd, code, SHORT_PACK_LEN);
    if (send_len <= 0)
    {
        printf("tcp_send error!\n");
        return false;
    }
    printf("tcp_send deviec:%d cmd :0x%x succeed   fd:%x!!!!!!!!!!!!!!!!!!!!!!!!!\n", send_to_device, cmd, info->dev_fd);
    return true;
}

bool network_long_package_data(tcp_device send_to_device, unsigned char cmd, unsigned char *code, unsigned char arg)
{
    tcp_network_info *info = NULL;
    if (send_to_device > TCP_DEVICE_OUTDOOR_2)
    {
        return false;
    }

    info = &net_info[send_to_device];

    if (info->dev_fd == -1)
    {
        return false;
    }
    unsigned char data[LONG_PACK_LEN] = {0};
    memcpy(&data[6], code, LONG_PACK_LEN - 7);
    data[0] = LONG_PACK_START;
    data[1] = LOCAL_DEVICE;
    data[2] = send_to_device;
    data[3] = cmd;
    data[4] = arg;
    data[LONG_PACK_LEN - 1] = PACK_END;

    int send_len = tcp_send(info->dev_fd, code, SHORT_PACK_LEN);
    if (send_len <= 0)
    {
        printf("tcp_send error!\n");
        return false;
    }
    return true;
}

bool network_file_package_data(tcp_device send_to_device, unsigned char cmd, unsigned char *file_data, unsigned int len, unsigned char arg, unsigned int fd)
{
    if (send_to_device > TCP_DEVICE_OUTDOOR_2)
    {
        return false;
    }

    unsigned char *data = NULL;
    int send_size = len + 8;
    data = malloc(send_size);

    if (data)
    {
        if (len > 0)
            memcpy(&data[7], file_data, len);

        data[0] = FILE_PACK_START;
        data[1] = LOCAL_DEVICE;
        data[2] = send_to_device;
        data[3] = cmd;
        data[4] = arg;
        data[5] = send_size & 0xFF;
        data[6] = send_size >> 8;
        data[send_size - 1] = PACK_END;
        int send_len = tcp_send(fd, data, send_size);
        if (send_len <= 0)
        {
            printf("tcp_send cmd 0x%x error!\n", cmd);
            free(data);
            return false;
        }

        free(data);
        printf("tcp_send cmd 0x%x data len :%d  succeed!\n", cmd, len);
        return true;
    }

    printf("tcp_send malloc fail !!!!!!!!!!!!!!!!!!\n");
    return false;
}
static void net_common_ack_func(const tcp_network_info *dev)
{
}
static void net_common_add_finger_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_FINGER)
        return;

    switch (dev->arg1)
    {
    case FAIL_RET:
        printf("%s===============================>>>FAIL_RET\n", __func__);
        tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
        break;
    case CONTINUE_RET:
        printf("%s===============================>>>CONTINUE_RET\n", __func__);
        break;
    case SUCCEE_RET:
        printf("%s===============================>>>SUCCEE_RET\n", __func__);
        tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
        break;
    case SATUTS_SWICTH:
        printf("%s===============================>>>SATUTS_SWICTH\n", __func__);
        break;

    default:
        break;
    }
}
static void net_common_exit_finger_func(const tcp_network_info *dev)
{
    // printf("%s===============================>>>%d\n",__func__,__LINE__);

    if (manage_type != MANAGE_FINGER)
        return;
}
static void net_common_del_finger_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_FINGER)
        return;

    switch (dev->arg1)
    {
    case FAIL_RET:
        printf("%s===============================>>>FAIL_RET\n", __func__);
        break;
    case CONTINUE_RET:
        printf("%s===============================>>>CONTINUE_RET\n", __func__);
        break;
    case SUCCEE_RET:
        tcp_network_event_push(dev->cmd, 0, dev->dev_id);
        printf("%s===============================>>>SUCCEE_RET\n", __func__);
        break;
    case SATUTS_SWICTH:
        printf("%s===============================>>>SATUTS_SWICTH\n", __func__);
        break;

    default:
        break;
    }
}
static void net_common_verify_finger_func(const tcp_network_info *dev)
{
    printf("%s===============================>>>%d\n", __func__, __LINE__);

    if (manage_type != MANAGE_FINGER)
        return;
}
static void net_common_get_finger_func(const tcp_network_info *dev)
{

    if (manage_type != MANAGE_FINGER)
        return;

    printf("The total number of finger :%d\n", dev->data[6]);
    bzero(data_packge_buffer, LONG_PACK_LEN);
    memcpy(data_packge_buffer, &(dev->data[6]), LONG_PACK_LEN - 7);
    tcp_network_event_push(dev->cmd, 0, dev->dev_id);
}

static void net_common_add_card_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_CARD)
        return;

    tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
    printf("%s===============================>>>%d:%d\n", __func__, __LINE__, dev->arg1);
}
static void net_common_exit_card_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_CARD)
        return;

    tcp_network_event_push(dev->cmd, 0, dev->dev_id);
    printf("%s===============================>>>%d\n", __func__, __LINE__);
}
static void net_common_del_card_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_CARD)
        return;

    tcp_network_event_push(dev->cmd, 0, dev->dev_id);
    printf("%s===============================>>>%d\n", __func__, __LINE__);
}
static void net_common_verify_card_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_CARD)
        return;

    tcp_network_event_push(dev->cmd, 0, dev->dev_id);
    printf("%s===============================>>>%d\n", __func__, __LINE__);
}
static void net_common_get_card_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_CARD)
        return;

    printf("The total number of cards :%d\n", dev->data[6]);
    bzero(data_packge_buffer, LONG_PACK_LEN);
    memcpy(data_packge_buffer, &(dev->data[6]), LONG_PACK_LEN - 7);
    tcp_network_event_push(dev->cmd, 0, dev->dev_id);

    // bool tcp_network_cmd_get_data_readme_send(tcp_device send_to_device);
    // tcp_network_cmd_get_data_readme_send(dev->dev_id);
}
static void net_common_access_denied_func(const tcp_network_info *dev)
{
    printf("NET_COMMON_CMD_ACCESS_DENIED !!!!!!!!!!!!!!!!!!!!!\n");
    tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
}
static void net_common_card_readme_receive_func(const tcp_network_info *dev)
{
    printf("<<<=================================card_readme_receive num :%d pack_len :%d=================================>>>n\n", dev->arg1, dev->data_size);
    static unsigned int receive_num = 0;
    static FILE *fp = NULL;
    unsigned int valid_data_len = dev->data_size - 8;
    if (fp == NULL)
    {
        if (dev->arg1 != 0)
        {
            printf("<<<=================================card_readme_receive ERROR 1=================================>>>n\n");
            return;
        }
        receive_num = 0;
        fp = fopen("/tmp/net_camera", "w+");

        extern unsigned long long os_get_ms(void);
        printf("<<<=================================card_readme_receive START=================================>>>%lld\n", os_get_ms());
    }

    if (fp == NULL)
    {
        printf("<<<=================================card_readme_receive FAIL!!!!!!! =================================>>>n\n");
        return;
    }

    if (valid_data_len >= 3072)
    {
        if (receive_num++ != dev->arg1)
        {
            printf("\n\n<<<XXXXXXXXXXXXXXXXXXXXXXXXXXXX=card_readme_receive packet loss:%d =XXXXXXXXXXXXXXXXXXXXXXXXXXXX>>>n\n\n", receive_num);
        }
        fwrite(&(dev->data[7]), 1, valid_data_len, fp);
    }
    else if (dev->arg1 == PACK_END)
    {
        receive_num = 0;
        fwrite(&(dev->data[7]), 1, valid_data_len, fp);
        fclose(fp);
        fp = NULL;
        extern unsigned long long os_get_ms(void);
        printf("<<<=================================card_readme_receive FINISH=================================>>>%lld\n", os_get_ms());
    }
    else
    {
        receive_num = 0;
        fclose(fp);
        fp = NULL;
        printf("<<<=================================card_readme_receive ERROR 2=================================>>>n\n");
        return;
    }
}
#if 0
static void card_remark_file_push(const tcp_network_info *dev)
{
    int data_len = 0;
    unsigned char data[READ_BUFFER_LEN - 8] = {0};
    FILE *fp = fopen("/tmp/net_camera","r");
    if(fp == NULL)
    {
        printf("\n\n<<<============================ open card_remark_file fail ============================>>>\n");
        return;
    }

    int package_num = 0;
    printf("\n\n<<<============================ card_remark Transmission start ============================>>>\n");
    while ((data_len = fread(data, sizeof(char),sizeof(data),fp)) > 0)
    {
        network_file_package_data(LOCAL_DEVICE,NET_COMMON_CMD_CARD_REMARK_RESPONSE,data,data_len,data_len < sizeof(data) ? PACK_END:package_num,dev->dev_fd);
        printf("card_remark package num:%d  data_len:%d\n",package_num++,data_len);
        ak_sleep_ms(1);
    }
    printf("<<<============================ card_remark Transmission completion ============================>>>\n\n\n");
}
#endif

static void net_common_add_passw_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_PASSW)
        return;

    tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
    printf("%s===============================>>>%d:%d\n", __func__, __LINE__, dev->arg1);
}

static void net_common_del_passw_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_PASSW)
        return;

    tcp_network_event_push(dev->cmd, dev->arg1, dev->dev_id);
    printf("%s===============================>>>%d:%d\n", __func__, __LINE__, dev->arg1);
}

static void net_common_get_passw_func(const tcp_network_info *dev)
{
    if (manage_type != MANAGE_PASSW)
        return;

    printf("The total number of password :%d\n", dev->data[6]);
    bzero(data_packge_buffer, LONG_PACK_LEN);
    memcpy(data_packge_buffer, &(dev->data[6]), LONG_PACK_LEN - 7);
    tcp_network_event_push(dev->cmd, 0, dev->dev_id);
    printf("%s===============================>>>%d:%d\n", __func__, __LINE__, dev->arg1);
}
static net_common_event_info net_common_event[] = {

    {"net common ack", NET_COMMON_CMD_ACK, net_common_ack_func},

    {"add finger status", NET_COMMON_CMD_ADD_FINGER, net_common_add_finger_func},

    {"exit finger status", NET_COMMON_CMD_EXIT_FINGER, net_common_exit_finger_func},

    {"del finger status", NET_COMMON_CMD_DEL_FINGER, net_common_del_finger_func},

    {"verify finger status", NET_COMMON_CMD_VERIFY_FINGER, net_common_verify_finger_func},

    {"get finger info", NET_COMMON_CMD_GET_FINGER, net_common_get_finger_func},

    {"add card status", NET_COMMON_CMD_ADD_CARD, net_common_add_card_func},

    {"exit card status", NET_COMMON_CMD_EXIT_CARD, net_common_exit_card_func},

    {"del card status", NET_COMMON_CMD_DEL_CARD, net_common_del_card_func},

    {"verify card status", NET_COMMON_CMD_VERIFY_CARD, net_common_verify_card_func},

    {"get card info", NET_COMMON_CMD_GET_CARD, net_common_get_card_func},

    {"card readme get", NET_COMMON_CMD_CARD_REMARK_RESPONSE, net_common_card_readme_receive_func},

    {"add password status", NET_COMMON_CMD_ADD_PASSW, net_common_add_passw_func},

    {"del password status", NET_COMMON_CMD_DEL_PASSW, net_common_del_passw_func},

    {"get password info", NET_COMMON_CMD_GET_PASSW, net_common_get_passw_func},

    {"access denied", NET_COMMON_CMD_ACCESS_DENIED, net_common_access_denied_func},

};

bool net_common_event_process(const tcp_network_info *dev)
{
    printf("receive dev :%d ,cmd %d \n", dev->dev_id, dev->cmd);
    int size = sizeof(net_common_event) / sizeof(net_common_event_info);
    int i = 0;
    for (i = 0; i < size; i++)
    {
        if (net_common_event[i].cmd == dev->cmd)
        {
            // device_info_display(device_info);
            printf("receive dev :%d ,cmd %s \n", dev->dev_id, net_common_event[i].str);
            net_common_event[i].proc(dev);
            break;
        }
    }
    return true;
}

typedef enum
{
    DATA_ERROR,
    DATA_VAILD,
    DATA_PARTIAL_PACKET,       /* 半包 */
    DATA_PACKET_CONCATENATION, /* 粘包 */

} check_vaild;
typedef struct
{
    check_vaild vaild_ret;
    char cmd;
    unsigned int vaild_data_len;
} vaild_data_ret;
typedef struct
{
    /* 接收缓存区首地址 */
    char *const receive_cache_addr;
    /* 当前处理数据首地址 */
    char *handle_buffer;
    /* 当前处理数据长度 */
    unsigned int handle_len;
    /* 下一份需要接收的数据大小 */
    unsigned int next_read_len;
    /* 下一份需要接收数据的缓存区地址偏移 */
    unsigned int cache_offset;
    /* 网络设备信息 */
    tcp_network_info *dev;
} receive_data_cache;

static vaild_data_ret net_common_code_check_valid(char *buffer, int ret, tcp_network_info *dev)
{
    printf("RECEIVE start :0x%x ,cmd :0x%x arg:0x%x\n", buffer[0], buffer[3], buffer[4]);
    vaild_data_ret check_ret;
    /*先判断起始码和结束码是否一致*/
    check_ret.cmd = buffer[0];
    switch (buffer[0])
    {
    case DATA_PACK_START:
    {
        check_ret.vaild_data_len = SHORT_PACK_LEN;
        break;
    }
    case LONG_PACK_START:
    {
        check_ret.vaild_data_len = LONG_PACK_LEN;
        break;
    }

    case FILE_PACK_START:
    {
        check_ret.vaild_data_len = (buffer[5] | (buffer[6] << 8));
        break;
    }

    default:
        check_ret.vaild_ret = DATA_ERROR;
        return check_ret;
    }

    if ((buffer[2] != LOCAL_DEVICE) && (buffer[2] != TCP_DEVICE_ALL))
    {
        check_ret.vaild_ret = DATA_ERROR;
        goto check_finish;
    }

    if (ret > check_ret.vaild_data_len)
    {
        if (buffer[check_ret.vaild_data_len - 1] == PACK_END)
        {
            /* 接收粘包 */
            check_ret.vaild_ret = DATA_PACKET_CONCATENATION;
            goto check_finish;
        }
        else
        {
            check_ret.vaild_ret = DATA_ERROR;
            goto check_finish;
        }
    }
    else if (ret < check_ret.vaild_data_len)
    {
        /* 接收半包 */
        check_ret.vaild_ret = DATA_PARTIAL_PACKET;
        goto check_finish;
    }
    else
    {
        /* 数据包完好，直接处理 */
        check_ret.vaild_ret = DATA_VAILD;
        goto check_finish;
    }

check_finish:

    return check_ret;
}

static void receive_data_handle(receive_data_cache *data)
{
    static vaild_data_ret ret;
    char *buf_p = data->handle_buffer;
    tcp_network_info *dev = data->dev;

    ret = net_common_code_check_valid(buf_p, data->handle_len, data->dev);
    switch (ret.vaild_ret)
    {
    case DATA_ERROR:
    {
        data->cache_offset = 0;
        break;
    }
    case DATA_VAILD:
    {
        data->cache_offset = 0;
        switch (ret.cmd)
        {
        case DATA_PACK_START:
        case LONG_PACK_START:
        {
            dev->cmd = buf_p[3];
            dev->arg1 = buf_p[4];
            dev->arg2 = buf_p[5];
            dev->data = buf_p;
            break;
        }
        case FILE_PACK_START:
        {
            printf("receive file package start !! package_len :%d  curr_ret:%d\n", dev->data_size, data->handle_len);
            dev->cmd = buf_p[3];
            dev->arg1 = buf_p[4];
            dev->data_size = ret.vaild_data_len;
            if (dev->data != NULL)
            {
                free(dev->data);
            }

            if ((dev->data = malloc(dev->data_size)) == NULL)
            {
                return;
            }
            memcpy(dev->data, buf_p, data->handle_len);
            break;
        }

        default:
            break;
        }
        net_common_event_process(dev);
        break;
    }
    case DATA_PARTIAL_PACKET:
    {
        /* 记录后半包数据长度 */
        data->next_read_len = ret.vaild_data_len - data->handle_len;
        data->cache_offset = data->handle_len;
        memmove(data->receive_cache_addr, data->handle_buffer, data->handle_len);
        printf("The data to be Handle is moved to the  first address of the cache,Wait for the next half pack  %d:%d :%p\n", data->next_read_len, data->cache_offset, data->receive_cache_addr);
        break;
    }
    case DATA_PACKET_CONCATENATION:
        switch (ret.cmd)
        {
        case DATA_PACK_START:
        case LONG_PACK_START:
        {
            printf("DATA_PACKET_CONCATENATION CLIENT_FD = %d  buf_p[0]:0x%x handle_len:%d\n", dev->dev_fd, buf_p[0], data->handle_len);
            dev->cmd = buf_p[3];
            dev->arg1 = buf_p[4];
            dev->arg2 = buf_p[5];
            data->handle_len -= ret.vaild_data_len;
            data->handle_buffer = &(data->handle_buffer[ret.vaild_data_len]);
            net_common_event_process(dev);

            receive_data_handle(data);
            break;
        }
        case FILE_PACK_START:
            /* code */
            break;

        default:
            break;
        }
        break;
    }
    return;
}
static void wati_tcp_process_close(const tcp_network_info *info)
{
    fd_set wfds;
    struct timeval tv;
    FD_ZERO(&wfds);
    FD_SET(info->dev_fd, &wfds);
    tv.tv_sec = 1; // 等待时间为1秒
    tv.tv_usec = 0;
    int ret = select(info->dev_fd + 1, NULL, &wfds, NULL, &tv);
    if (ret < 0)
    {
        perror("select");
    }
    else if (ret == 0)
    {
        printf("timeout\n");
    }
}

static bool tcp_management_run = false;
void *tcp_management_task(void *arg)
{
    tcp_network_info *info = ((tcp_network_info *)arg);

    char buf[READ_BUFFER_LEN] = {0};
    receive_data_cache data = {.dev = info,
                               .handle_buffer = buf,
                               .receive_cache_addr = buf,
                               .next_read_len = READ_BUFFER_LEN,
                               .cache_offset = 0};

    if (info->dev_id) // 避免ARP缓存中的MAC地址因门口机id切换与之前IP不相符，每次连接前清除ARP缓存
    {
        system("arp -d " OUTDOOR2_IP);
    }
    else
    {
        system("arp -d " OUTDOOR1_IP);
    }

    tcp_management_run = true;

    printf("======[ TCP_CONNECT DOOR:%d THREAD:%lu  WATI...............    !!!!! ]======\n", info->dev_id, info->thread_id);

    info->dev_fd = tcp_connect(info->dev_id ? OUTDOOR2_IP : OUTDOOR1_IP, TCP_PORT);
    if (info->dev_fd < 0)
    {
        printf("tcp_connect error thread_id :%lu!\n", info->thread_id);
        info->thread_id = 0;

        if ((net_info[TCP_DEVICE_OUTDOOR_1].thread_id | net_info[TCP_DEVICE_OUTDOOR_2].thread_id) == 0)
            tcp_management_run = false;

        ak_thread_exit();
        return NULL;
    }

    /*     tcp_connec阻塞时手动关闭，阻塞结束后退出线程,虽然第608行进行了置位true，
            但是connect阻塞的过程中，手动退出调用tcp_management_close函数置位为false,
            阻塞结束自动退出线程
     */
    if (tcp_management_run == false)
    {
        goto exit;
    }

    printf("======[ TCP_CONNECT DOOR:%d SUCCEED   fd:0x%x !!!!! ]======\n", info->dev_id, info->dev_fd);
    tcp_network_cmd_get_data_info_send(info->dev_id);

    while (tcp_management_run)
    {
        if (/* scanf("%s", buf) */ 1)
        {
            // bzero(buf, sizeof(buf));
            int recv_len = tcp_nonblocking_recv(info->dev_fd, &(data.receive_cache_addr[data.cache_offset]), data.next_read_len, 0, 5000);
            if (recv_len == 0)
            {
                printf("tcp_blocking_recv exit!\n");
                break;
            }

            if (recv_len > 0)
            {
                printf("recv_len]======%d\n", recv_len);
            }

            if (recv_len > 0)
            {
                data.handle_len = recv_len + data.cache_offset;
                data.next_read_len = READ_BUFFER_LEN;
                data.handle_buffer = data.receive_cache_addr;
                extern unsigned long long os_get_ms(void);
                unsigned long long start_ms = os_get_ms();
                printf("recv_len ============>>%d:%p\n", recv_len, data.receive_cache_addr);
                receive_data_handle(&data);
                printf("<<=====================Complete event handleing Processing time:%llu ms============>>\n\n\n", os_get_ms() - start_ms);
            }
            // printf("recv : %s\n", buf);
        }
        // usleep(5000*1000);
    }

exit:

    // card_remark_file_push(info);

    // 等待输出缓冲区变为空闲状态
    wati_tcp_process_close(info);

    tcp_close(info->dev_fd);
    info->dev_fd = -1;

    printf("======[ TCP MANAGRMENT DOOR:%d EXIT FINISH    !!!! ]======\n", info->dev_id);
    ak_thread_exit();
    return NULL;
}

void tcp_network_info_init(void)
{
    for (int i = 0; i < TCP_DEVICE_END; i++)
    {
        net_info[i].arg1 = 0;
        net_info[i].arg2 = 0;
        net_info[i].cmd = 0;
        net_info[i].data = NULL;
        net_info[i].data_size = 0;
        net_info[i].dev_fd = -1;
        net_info[i].dev_id = i;
        net_info[i].thread_id = 0;
    }
}

bool tcp_management_init(tcp_device device_id)
{

    if (net_info[device_id].thread_id == 0)
    {
        ak_thread_create(&(net_info[device_id].thread_id), tcp_management_task, &net_info[device_id], ANYKA_THREAD_MIN_STACK_SIZE, -1);
    }
    else
    {
        printf("management dev :%d create fail ......... %lu\n", device_id, net_info[device_id].thread_id);
        return false;
    }

    return true;
}

bool tcp_management_close(void)
{
    if (tcp_management_run == false)
    {
        return false;
    }
    tcp_management_run = false;

    if (net_info[TCP_DEVICE_OUTDOOR_1].thread_id != 0)
    {
        ak_thread_join(net_info[TCP_DEVICE_OUTDOOR_1].thread_id);
        net_info[TCP_DEVICE_OUTDOOR_1].thread_id = 0;
    }

    if (net_info[TCP_DEVICE_OUTDOOR_2].thread_id != 0)
    {
        ak_thread_join(net_info[TCP_DEVICE_OUTDOOR_2].thread_id);
        net_info[TCP_DEVICE_OUTDOOR_2].thread_id = 0;
    }

    tcp_network_info_init();
    printf("================>>> tcp线程退出\n");
    return true;
}

bool tcp_network_cmd_get_data_readme_send(tcp_device send_to_device)
{
    int cmd = 0x00;
    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_CARD_REMARK_PULL;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_FINGER_REMARK_GET;
    }
    else
    {
        return false;
    }

    return network_short_send_common(send_to_device, cmd, 0, 0);
}

bool tcp_network_cmd_get_data_info_send(tcp_device send_to_device)
{
    int cmd = 0x00;
    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_GET_CARD;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_GET_FINGER;
    }
    else if (manage_type == MANAGE_PASSW)
    {
        cmd = NET_COMMON_CMD_GET_PASSW;
    }
    else
    {
        return false;
    }

    return network_short_send_common(send_to_device, cmd, 1, 2);
}

bool tcp_network_cmd_add_data_send(tcp_device send_to_device, unsigned char arg1, unsigned char arg2)
{
    int cmd = 0x00;

    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_ADD_CARD;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_ADD_FINGER;
    }
    else if (manage_type == MANAGE_PASSW)
    {
        cmd = NET_COMMON_CMD_ADD_PASSW;
        /* LINK */
    }
    else
    {
        return false;
    }
    return network_short_send_common(send_to_device, cmd, arg1, arg2);
}

bool tcp_network_cmd_exit_send(tcp_device send_to_device)
{
    int cmd = 0x00;
    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_EXIT_CARD;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_EXIT_FINGER;
    }
    else
    {
        return false;
    }
    return network_short_send_common(send_to_device, cmd, 0, 0);
}

bool tcp_network_cmd_del_data_send(int index, tcp_device send_to_device)
{
    int cmd = 0x00;
    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_DEL_CARD;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_DEL_FINGER;
    }
    else if (manage_type == MANAGE_PASSW)
    {
        cmd = NET_COMMON_CMD_DEL_PASSW;
    }
    else
    {
        return false;
    }
    return network_short_send_common(send_to_device, cmd, index, 1);
}

bool tcp_network_cmd_set_lock_type_send(int index, uint8_t lock_type, tcp_device send_to_device)
{
    int cmd = 0x00;
    if (manage_type == MANAGE_CARD)
    {
        cmd = NET_COMMON_CMD_SET_CARD_PERMISSION;
    }
    else if (manage_type == MANAGE_FINGER)
    {
        cmd = NET_COMMON_CMD_SET_FINGER_PERMISSION;
    }
    else if (manage_type == MANAGE_PASSW)
    {
        cmd = NET_COMMON_CMD_SET_PASSW_PERMISSION;
    }
    else
    {
        return false;
    }
    return network_short_send_common(send_to_device, cmd, index, lock_type);
}

management_type data_manage_type_get(void)
{
    return manage_type;
}

void data_manage_type_set(management_type type)
{
    manage_type = type;
}

char *data_packge_buffer_get(void)
{
    return data_packge_buffer;
}

bool management_create_state(tcp_device dev)
{
    return net_info[dev].thread_id == 0 ? false : true;
}