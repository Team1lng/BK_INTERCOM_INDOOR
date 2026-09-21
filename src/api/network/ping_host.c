#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <net/if.h>
#include <stdio.h>
#include <time.h>
#include <linux/errno.h>
/**
 * @description:  打印调用system执行的命令, 可以按printf格式输入
 * @date   2023-11-21 20:38:25
 * @author xiaole
 * @note
 */
#define system_exe(format, ...)                                \
    do                                                         \
    {                                                          \
        char buf[512] = {0};                                   \
        snprintf(buf, sizeof(buf) - 1, format, ##__VA_ARGS__); \
        printf("%s\n", buf);                                   \
        system(buf);                                           \
    } while (0);

/**
 * @description: 获取时间差
 * @date   2023-07-08 10:21
 * @author xiaole
 * @param {timespec} *last_time 基准时间
 * @return {*}  时间差值
 */
unsigned long long DiffClockTimeMs(struct timespec *last_time)
{
    struct timespec curr_time;
    unsigned long long diff;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    diff = (curr_time.tv_sec - last_time->tv_sec) * 1000 + (curr_time.tv_nsec - last_time->tv_nsec) / 1000000;

    return diff;
}

/**
 * @description: 获取时间
 * @date   2023-07-08 10:21
 * @author xiaole
 * @param {timespec} *time 时间缓存
 * @return {*}  时间差值
 */
void GetClockTimeMs(struct timespec *time)
{
    clock_gettime(CLOCK_MONOTONIC, time);
    return;
}

static int Wlan0DeviceCheak(void)
{
    struct ifreq ifr;
    int sockfd;
    // 创建一个套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    // 配置 ifreq 结构体
    strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ - 1);

    // 使用 ioctl 检查设备是否存在
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1)
    {
        perror("ioctl");
        close(sockfd);
        return -1;
    }

    // 设备存在
    close(sockfd);
    // printf("Device %s is loaded.\n", "wlan0");
    return 0;
}
/*************************************************************************
 * @description:  检查wlan0设备状态
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
    [1]:连接
    [0]:非连接
 * @param
 **************************************************************************/
int Wlan0WapStateCheck(void)
{
#define MAX_COMMAND 100
#define MAX_STATE_LENGTH 20
    FILE *fp;
    char command[MAX_COMMAND];
    char state[MAX_STATE_LENGTH];
    char *result;

    // 构建执行命令
    snprintf(command, sizeof(command), "wpa_cli -i wlan0 status | grep 'wpa_state=' | cut -d= -f2");

    // 执行命令并读取输出
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        return -1;
    }

    // 读取命令输出
    if (fgets(state, sizeof(state), fp) == NULL)
    {
        printf("Failed to read output\n");
        return -1;
    }

    // 关闭文件指针
    pclose(fp);

    // 去除末尾可能的换行符
    if ((result = strchr(state, '\n')) != NULL)
    {
        *result = '\0';
    }

    return (strcmp(state, "COMPLETED") == 0 || strcmp(state, "ASSOCIATED") == 0);
}

/*************************************************************************
 * @description:  获取指定网卡的默认网关
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
    成功：1
    失败：0
 * @param
    interface:网卡接口
    gateway:网关地址暂存区
 **************************************************************************/
int GetDefaultGateway(const char *interface, char *gateway)
{
    char command[MAX_COMMAND];
    char line[MAX_COMMAND];
    FILE *fp;

    // 构建要执行的命令
    snprintf(command, sizeof(command), "ip route show dev %s | awk '/default/ {print $3}'", interface);

    // 执行命令并打开管道获取输出
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        return 0;
    }

    // 读取命令输出，获取网关地址
    if (fgets(line, sizeof(line) - 1, fp) != NULL)
    {
        // 去除可能的换行符
        line[strcspn(line, "\n")] = 0;
        strncpy(gateway, line, sizeof(line));
    }

    pclose(fp);
    return 1;
}

/*************************************************************************
 * @description:  获取wlan0扫描热点个数
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
    成功：1
    失败：0
 * @param
 **************************************************************************/
int CountWifiNetworks(void)
{
    FILE *fp;
    char command[MAX_COMMAND];
    char line[MAX_COMMAND];
    int count = 0;

    // 执行wpa_cli scan命令
    snprintf(command, sizeof(command), "wpa_cli -i wlan0 scan > /dev/null && sleep 1 && wpa_cli -i wlan0 scan_results | tail -n +2");

    // 打开管道并执行命令
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("Failed to execute command");
        return -1;
    }

    // 读取命令输出，统计WiFi热点数量
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        count++;
    }

    // 关闭管道
    pclose(fp);

    // 返回WiFi热点数量
    return count;
}
/*************************************************************************
 * @description:  Ping 指定网卡默认网关
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
    [0]:成功
    [1]:失败
 * @param
    interface:网卡接口
 **************************************************************************/
int PingGateway(const char *interface)
{
#define PING_TIMEOUT 2 // Ping 超时时间，单位为秒
    int ret;
    char command[MAX_COMMAND];
    char gateway[20];

    GetDefaultGateway(interface, gateway);
    snprintf(command, sizeof(command), "ping -I %s -c 1 -W %d %s > /dev/null 2>&1", interface, PING_TIMEOUT, gateway);

    ret = system(command);
    return ret;
}

/*************************************************************************
 * @description:  检测网络通信状态
 * @date   2023-07-08 10:21
 * @author xiaole
 * @return
 * [-1]:异常
    [0]:过渡
    [1]:连接正常
    [2]:未连接
 * @param
    [interval]:检测间隔，毫秒单位
    [fail_count]:错误次数，达到执行error_handler
    [fail_handler]:错误处理函数
 **************************************************************************/
int Wlan0HealthCheck(int interval, int fail_count, void (*fail_handler)(void *))
{
    static struct timespec time;
    static int handler_flag = 0; /* 异常处理标志：若执行异常处理后，作为下一次异常处理判断条件，当网络通讯正常时清零 */
    static int fail = 0;

    /* 间隔 interval 毫秒进入一次检测*/
    if (DiffClockTimeMs(&time) > interval)
    {
        GetClockTimeMs(&time);
        if (Wlan0DeviceCheak())
        {
            return -1;
        }

        /* 判断wlan0连接状态，若为连接网络则退出检测 */
        if (!Wlan0WapStateCheck())
        {
            printf("WiFi is disconnected.\n");
            return 2;
        }

        /* 尝试Ping Wlan0网卡默认网关 */
        if (PingGateway("wlan0"))
        {
            printf("Failed to ping gateway. WiFi connection might be unstable.fail:%d\n", fail);
            /* Ping失败达到一定次数 */
            if ((++fail) >= fail_count)
            {
                fail = 0;
                /* 若网络异常及未进行过异常处理，且wifi热点个数仅一个 */
                if (!handler_flag && CountWifiNetworks() <= 1)
                {
                    if (fail_handler)
                    {
                        /* 异常处理标志 */
                        handler_flag = 1;
                        fail_handler(NULL);
                    }
                }
            }
            return -1;
        }
        /* 网络通讯正常，清除异常标志 */
        fail = handler_flag = 0;
        return 1;
    }
    return 0;
}