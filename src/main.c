#if 0

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

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

struct block_desc
{
    uint32_t version;
    uint32_t offset_to_priv;
    struct tpacket_hdr_v1 h1;
};

struct ring
{
    struct iovec *rd;
    uint8_t *map;
    struct tpacket_req3 req;
};

static unsigned long packets_total = 0, bytes_total = 0;
static sig_atomic_t sigint = 0;

static void sighandler(int num)
{
    sigint = 1;
}

/* 初始化套接字，包括套接口创建、接收缓冲区的创建等 */
static int setup_socket(struct ring *ring, char *netdev)
{
    int err, i, fd, v = TPACKET_V3;
    struct sockaddr_ll ll;
    unsigned int blocksiz = 4096;//1 << 22,
    unsigned int framesiz = 2048;//1 << 11;
    unsigned int blocknum = 64;

    /* 创建套接口 */
    fd = socket(AF_PACKET, SOCK_RAW, htons(0xFFFF));
    if (fd < 0)
    {
        perror("socket");
        exit(1);
    }

    /* 设置PACKET版本，有v1、v2和v3三个版本，默认是v1 */
    err = setsockopt(fd, SOL_PACKET, PACKET_VERSION, &v, sizeof(v));
    if (err < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    memset(&ring->req, 0, sizeof(ring->req));
    ring->req.tp_block_size = blocksiz;
    ring->req.tp_frame_size = framesiz;
    ring->req.tp_block_nr = blocknum;
    ring->req.tp_frame_nr = (blocksiz * blocknum) / framesiz;
    ring->req.tp_retire_blk_tov = 60;
    ring->req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

    /* 创建ringBuf */
    err = setsockopt(fd, SOL_PACKET, PACKET_RX_RING, &ring->req, sizeof(ring->req));
    if (err < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    /* 将ringBuf映射到用户态 */
    ring->map = mmap(NULL, ring->req.tp_block_size * ring->req.tp_block_nr, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);
    if (ring->map == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    /* 使用iovec向量的方式来访问缓冲区，为每个块创建一个向量，存储到ring->rd中 */
    ring->rd = malloc(ring->req.tp_block_nr * sizeof(*ring->rd));
    assert(ring->rd);
    /* 初始化向量，使用与每个块对应 */
    for (i = 0; i < ring->req.tp_block_nr; ++i)
    {
        ring->rd[i].iov_base = ring->map + (i * ring->req.tp_block_size);
        ring->rd[i].iov_len = ring->req.tp_block_size;
    }

    memset(&ll, 0, sizeof(ll));
    ll.sll_family = PF_PACKET;
    ll.sll_protocol = htons(ETH_P_ALL);
    ll.sll_ifindex = if_nametoindex(netdev);
    ll.sll_hatype = 0;
    ll.sll_pkttype = 0;
    ll.sll_halen = 0;

    /* 将这个原始套接字绑定到某个网口（netdev） */
    err = bind(fd, (struct sockaddr *) &ll, sizeof(ll));
    if (err < 0)
    {
        perror("bind");
        exit(1);
    }

    return fd;
}

/* 显示报文数据 */
static void display(struct tpacket3_hdr *ppd)
{
    uint8_t* buf = (uint8_t*)((uint8_t *)ppd + ppd->tp_mac);
    for(int i = 0 ; i < 68 ; i++)
    {
        printf("%02x ",buf[i]);
    }
    printf("\n");
    return ;


    /* 帧头部的地址加上MAC偏移量，就是以太网报文的地址 */
    struct ethhdr *eth = (struct ethhdr *) ((uint8_t *) ppd + ppd->tp_mac);
    struct iphdr *ip = (struct iphdr *) ((uint8_t *) eth + ETH_HLEN);

    if (eth->h_proto == htons(0xFFFF))
    {
        struct sockaddr_in ss, sd;
        char sbuff[NI_MAXHOST], dbuff[NI_MAXHOST];

        memset(&ss, 0, sizeof(ss));
        ss.sin_family = PF_INET;
        ss.sin_addr.s_addr = ip->saddr;
        /* 将源IP地址转换成主机名字 */
        getnameinfo((struct sockaddr *) &ss, sizeof(ss), sbuff, sizeof(sbuff), NULL, 0, NI_NUMERICHOST);

        memset(&sd, 0, sizeof(sd));
        sd.sin_family = PF_INET;
        sd.sin_addr.s_addr = ip->daddr;
        getnameinfo((struct sockaddr *) &sd, sizeof(sd), dbuff, sizeof(dbuff), NULL, 0, NI_NUMERICHOST);

        /* 打印出来地址信息 */
        printf("%s -> %s, ", sbuff, dbuff);
    }

    printf("rxhash: 0x%x\n", ppd->hv1.tp_rxhash);
}

static void walk_block(struct block_desc *pbd, const int block_num)
{
    int num_pkts = pbd->h1.num_pkts, i;
    unsigned long bytes = 0;
    struct tpacket3_hdr *ppd;

    /* 获取当前块中第一个帧 */
    ppd = (struct tpacket3_hdr *) ((uint8_t *) pbd + pbd->h1.offset_to_first_pkt);
    for (i = 0; i < num_pkts; ++i)
    {
        bytes += ppd->tp_snaplen;
        display(ppd);

        /* 获取下一个帧的位置 */
        ppd = (struct tpacket3_hdr *) ((uint8_t *) ppd + ppd->tp_next_offset);
    }

    packets_total += num_pkts;
    bytes_total += bytes;
}

static void flush_block(struct block_desc *pbd)
{
    pbd->h1.block_status = TP_STATUS_KERNEL;
}

static void teardown_socket(struct ring *ring, int fd)
{
    /* 销毁套接字 */
    munmap(ring->map, ring->req.tp_block_size * ring->req.tp_block_nr);
    free(ring->rd);
    close(fd);
}

int main(int argc, char **argp)
{
    int fd, err;
    socklen_t len;
    struct ring ring;
    struct pollfd pfd;
    unsigned int block_num = 0, blocks = 64;
    struct block_desc *pbd;
    struct tpacket_stats_v3 stats;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s INTERFACE\n", argp[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, sighandler);

    memset(&ring, 0, sizeof(ring));
    /* 初始化套接字 */
    fd = setup_socket(&ring, argp[argc - 1]);
    assert(fd > 0);

    /* 初始化poll参数 */
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    pfd.events = POLLIN | POLLERR;
    pfd.revents = 0;

    /* 进入poll的循环收包环节 */
    while (likely(!sigint))
    {
        pbd = (struct block_desc *) ring.rd[block_num].iov_base;

        /* 检查当前块头的状态，判断是否有数据，没有的话就进行poll */
        if ((pbd->h1.block_status & TP_STATUS_USER) == 0)
        {
            poll(&pfd, 1, -1);
            continue;
        }

        /* 有数据，遍历块里面的帧 */
        walk_block(pbd, block_num);
        /* 将块恢复为就绪状态 */
        flush_block(pbd);
        block_num = (block_num + 1) % blocks;
    }

    len = sizeof(stats);
    /* 获取报文统计信息，然后打印出来。 */
    err = getsockopt(fd, SOL_PACKET, PACKET_STATISTICS, &stats, &len);
    if (err < 0)
    {
        perror("getsockopt");
        exit(1);
    }

    fflush(stdout);
    printf("\nReceived %u packets, %lu bytes, %u dropped, freeze_q_cnt: %u\n", stats.tp_packets, bytes_total, stats.tp_drops, stats.tp_freeze_q_cnt);

    teardown_socket(&ring, fd);
    return 0;
}

#else
#include "lvgl.h"
#include <stdio.h>
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ak_thread.h"
#include "ak_mem.h"

#include "layout_define.h"
#include "leo_api.h"
#include "user_data.h"

#include <stdlib.h>
#include <sys/signal.h>

// 默认读写一个关闭的socket会触发sigpipe信号 该信号的默认操作是关闭进程 有时候这明显是我们不想要的
// 所以此时我们需要重新设置sigpipe的信号回调操作函数  比如忽略操作等  使得我们可以防止调用它的默认操作
// 信号的处理是异步操作 也就是说 在这一条语句以后继续往下执行中如果碰到信号依旧会调用信号的回调处理函数
// 处理sigpipe信号

void handle_pipe(int sig)
{
    printf("%s !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", __func__);
    fflush(stdout);
}
void pipe_info_func(int signo, siginfo_t *info, void *p)
{
    printf("\n\n%s !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", __func__);
    printf("signo=%d\n", signo);

    printf("sender sigal pid=%d\n", info->si_pid);
    printf("%s !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n", __func__);
}
void handle_for_sigpipe()
{
    struct sigaction sa; // 信号处理结构体
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN; // 设置信号的处理回调函数 这个SIG_IGN宏代表的操作就是忽略该信号
    sa.sa_flags = 0;
    // sa.sa_sigaction = pipe_info_func;
    if (sigaction(SIGPIPE, &sa, NULL)) // 将信号和信号的处理结构体绑定
        return;
}

/**
 *  @brief print memory useage state
 *  @param[in] void
 *  @returnval  void
 */
void memory_print(void)
{
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d used_cnt:%d, max_used = %d free_size:%d free_cnt:%d ,total_size:%d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size, mon.used_cnt, mon.max_used, mon.free_size, mon.free_cnt, mon.total_size);
}

static void *lvgl_titck_task(void *arg)
{
    struct ak_timeval tv1, tv2;

    ak_get_ostime(&tv1);
    ak_get_ostime(&tv2);
    while (1)
    {
        ak_get_ostime(&tv1);
        lv_tick_inc(tv1.sec * 1000 + tv1.usec / 1000 - tv2.sec * 1000 - tv2.usec / 1000);
        ak_get_ostime(&tv2);
        ak_sleep_ms(1);
    }
    ak_thread_exit();
    return NULL;
}

static void standby_timeout_callback(void)
{
    // Debug("==========================================================================\n");
    ring_init();

    if (current_layout_get() != &layout_standby &&
        current_layout_get() != &layout_monitor &&
        // current_layout_get() != &layout_call &&
        current_layout_get() != &layout_video &&
        current_layout_get() != &layout_tuya_register &&
        // current_layout_get() != &layout_setting_senior &&
        current_layout_get() != &layout_cctv &&
        current_layout_get() != &layout_dev_busy &&
        current_layout_get() != &layout_time_display)
    {
        Debug("\n\n\n\n");
        goto_layout(pLAYOUT(standby));
    }
}

#define ETH2_STATIC_IP "192.168.188.1"
static void *network_pairing_init_task(void *arg)
{
    Debug("pairing_mode ,%d\n", user_data_get()->pairing_mode);
    /*     if(user_data_get()->pairing_mode == WLAN_NET)
        {
            system("ifconfig eth2 down");

            system("ifconfig wlan0 up");
        }
        else */
    if (user_data_get()->pairing_mode == WIRED_NET)
    {
        system("ifconfig eth2 up");
        Debug("ifconfig eth2 up");
        ak_sleep_ms(100);

        system("ifconfig wlan0 down");
        Debug("ifconfig wlan0 down");
        ak_sleep_ms(100);

        system("killall udhcpc");
        Debug("killall udhcpc");
        ak_sleep_ms(1000);

        if (user_data_get()->allocation_mode == UDHCPC_ALLOC)
        {
            system("udhcpc -i eth2 &");
            Debug("udhcpc -i eth2 &");
            ak_sleep_ms(100);
        }
        else if (user_data_get()->allocation_mode == STATIC_ALLOC)
        {
            system("ifconfig eth2 192.168.188.1");
            Debug("ifconfig eth2 192.168.188.1");
            ak_sleep_ms(100);
        }

        system("route add -net 224.0.0.0 netmask 224.0.0.0 eth2");
        Debug("route add -net 224.0.0.0 netmask 224.0.0.0 eth2\n");
    }

    tuya_network_dev_set(&user_data_get()->pairing_mode);
    *((ak_pthread_t *)arg) = -1;
    ak_thread_exit();
    return NULL;
}

static void network_pairing_int(void)
{
    if (wifi_usb_module_enable())
    {
        static ak_pthread_t pthread_id = -1;
        if (pthread_id == -1)
            ak_thread_create(&pthread_id, network_pairing_init_task, &pthread_id, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    }
}

/*
 *   这段函数是为了实现卡片指纹管理功能，旧的ak_eth.ko不能同时用ICMP协议通讯两个PHY
 *   需要将添加了混杂模式的ko重新加载一次
 * */
static void ak_eth_reload(void)
{
#ifdef BCOM_OID_VERSION

#define AK_ETH_PATH_1 "/tmp/ak_eth.ko"
#define AK_ETH_PATH_2 "/etc/config/ak_eth.ko"

    if (access(AK_ETH_PATH_1, F_OK) == 0)
    {
        Debug("fine %s ,reload now!\n", AK_ETH_PATH_1);
        system("rmmod ak_eth.ko");
        ak_sleep_ms(200);
        system("insmod " AK_ETH_PATH_1 "  yt8510_rate_model=0x01");
    }
    else if (access(AK_ETH_PATH_2, F_OK) == 0)
    {
        Debug("fine %s ,reload now!\n", AK_ETH_PATH_2);
        system("rmmod ak_eth.ko");
        ak_sleep_ms(200);
        system("insmod " AK_ETH_PATH_2);
    }
#endif
}

void main_device_monitor_busy_func(unsigned long arg1, unsigned long arg2)
{
    if (current_layout_get() != &layout_standby && monitor_enter_way_get() != MONITOR_ENTER_TUYA)
    {
        Debug("\n\n\n\n");
        goto_layout(pLAYOUT(standby));
    }
}

int lcd_reset_pin_higt(void)
{
    system("echo 34 > /sys/class/gpio/export");
    system("echo 1 > /sys/class/gpio/gpio34/value");
    return 1;
}

static lv_task_t *door_chime_det_task_t = NULL;
int main(int argv, char **argc)
{
    // int retrieve_all_upgrade_path(void);
    // retrieve_all_upgrade_path();
    // return;
    // char * buffer = malloc(1000*1000);
    // if(buffer)
    //     printf("Example Apply for one M memory\n");

    // // free(buffer);
    // return 0;

    Debug("INDOOR VERSION:%s-%s\n", __DATE__, __TIME__);
    system("echo 0 > /proc/sys/vm/oom_dump_tasks");
    system("hwclock -w");
    handle_for_sigpipe();

    ak_eth_reload();

    void upgrade_check_firmware(void);
    upgrade_check_firmware();

    network_pairing_int();
    /* 必须放在wifi、usb初始化后面，此线程中也wifi操作 */
    user_data_init();

    lv_init();            // lvgl 系统初始化
    lv_port_disp_init();  // lvgl 显示接口初始化,放在 lv_init()的后面
    lv_port_indev_init(); // lvgl 输入接口初始化,放在 lv_init()的后面

    leo_api_init();

    // extern void PWM1_AVDD(void);
    // PWM1_AVDD();
    network_devices_enable_init();

    device_monitor_busy_register(main_device_monitor_busy_func);

    device_gate2_unlock_register(default_gate2_unlock_callback);

    device_id_repeat_register(device_id_repeat_func);

    door_chime_event_register(door_chime_func);
    // goto_layout(pLAYOUT(home));

    standby_timer_open(60000, standby_timeout_callback);

    ak_pthread_t pthread_id;
    ak_thread_create(&pthread_id, lvgl_titck_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

    extern void hardware_detect_task(lv_task_t * task_t);
    door_chime_det_task_t = lv_task_create(hardware_detect_task, 50, LV_TASK_PRIO_MID, NULL);

    // extern void printf_str(void);
    // printf_str();
    // int count = 0;
    struct ak_timeval tv1;
    bool speak_status = false;
    speak_enable_set(0);
    while (1)
    {
        lv_task_handler();
        ak_get_ostime(&tv1);
        struct ak_timeval tv2 = audio_output_time_get();
        if (speak_status)
        {
            if (abs(tv1.sec - tv2.sec) > 3)
            {
                speak_enable_set(0);
                speak_status = false;
            }
        }
        else
        {
            if (abs(tv1.sec - tv2.sec) < 3)
            {
                speak_enable_set(1);
                speak_status = true;
            }
        }
        ak_sleep_ms(1);
        // count++;
        // if(count > 100)
        // {
        // 	count = 0;
        // system("sync");
        // system("echo 3 > /proc/sys/vm/drop_caches");
        // system("free");
        // memory_print();
        //  }
    }
}
#endif
