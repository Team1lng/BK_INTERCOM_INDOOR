#include <stdbool.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "string.h"
// #include "user_data.h"
extern void *osal_fb_mmap_viraddr(int fb_len, int fb_fd);
/*
dev:    size   erasesize  name
mtd0: 00037000 00001000 "UBOOT"
mtd1: 00001000 00001000 "ENV"
mtd2: 00001000 00001000 "ENVBK"
mtd3: 00010000 00001000 "DTB"
mtd4: 00190000 00001000 "KERNEL"
mtd5: 00025000 00001000 "LOGO"
mtd6: 001b6000 00001000 "ROOTFS"
mtd7: 0097e000 00001000 "CONFIG"
mtd8: 002ce000 00001000 "APP"
*/

#define FB_PATH "/dev/fb0"
#define FIRMWARE_PATH "/mnt/tf/firmware/"
#define SD_PATH "/mnt/tf/"

#define UBOOT	"u-boot.bin"
#define ENV	"env_ak3761e_nor.img"
#define DTB	"EVB_CBDM_AK3760E_V1.0.1.dtb"
#define KERNEL	"uImage"
#define LOGO "anyka_logo.rgb"
#define ROOTFS	"root.sqsh4"
#define CONFIG	"usr.jffs2"
#define APP	"usr.sqsh4"

#define UBOOT_SIZE  0x37000
#define ENV_SIZE 	0x1000
#define DTB_SIZE 	0x10000
#define KERNEL_SIZE		0x19c000
#define LOGO_SIZE 	0x19000
#define ROOTFS_SIZE 	0x263000
#define CONFIG_SIZE 	0x8ca000
#define APP_SIZE 	0x2d5000

#define APP_SOFTWARE	"two_wire_indoor.update"
static int kernel_upgrade_total = 0;
static int kernel_upgrade_count = 0;

static unsigned char *fb_adder = NULL;
static struct fb_var_screeninfo var_info;
static struct fb_fix_screeninfo fix_info;

static bool fb_init(void)
{
	// system("rmmod /usr/modules/ak_fb.ko");
	// system("insmod /usr/modules/ak_fb.ko");
	// system("insmod /usr/modules/ak_gui.ko");
	// system("rmmod ak_fb");
	// system("insmod /usr/modules/ak_fb.ko");

	// system("fbset -fb /dev/fb0 -g 1024 600 1024 600 24");

	// system("rmmod ak_fb");
	// system("insmod /usr/modules/ak_fb.ko");

	// system("fbset -fb /dev/fb0 -g 1024 600 1024 600 24");

	int fd = open(FB_PATH, O_RDWR);
	if (fd < 0)
	{
		printf("open %s failed \n", FB_PATH);
		return false;
	}
	/***** 获取fb的相关的信息 *****/

	ioctl(fd, FBIOGET_VSCREENINFO, &var_info);

	var_info.activate |= FB_ACTIVATE_FORCE;
	var_info.activate |= FB_ACTIVATE_NOW;
	var_info.xres = var_info.xres_virtual;
	var_info.yres = var_info.yres_virtual;
	/***** 设置RGB565格式 *****/
	var_info.bits_per_pixel = 24; // 16;
	var_info.red.offset = 16;     // 11;
	var_info.red.length = 8;      // 5;
	var_info.green.offset = 8;    // 5;
	var_info.green.length = 8;    // 6;
	var_info.blue.offset = 0;     // 0;
	var_info.blue.length = 8;     // 5;
	ioctl(fd, FBIOPUT_VSCREENINFO, &var_info);
	printf("var_info.xres_virtual:%d     var_info.yres_virtual:%d\n",var_info.xres_virtual,var_info.yres_virtual);
	/***** 获取不可变参数 *****/
	ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);

	/***** 初始化tde 需要的fb layer ******/
	fb_adder = (unsigned char *)osal_fb_mmap_viraddr(fix_info.smem_len, fd);
	memset((void *)fb_adder, 0x00, fix_info.smem_len);
	// var_info.reserved[0] = 0;
	// ioctl(fd, FBIOPUT_VSCREENINFO, &var_info);
	return true;
}

/***
** 日期: 2022-05-18 15:18
** 作者: leo.liu
** 函数作用：画矩形
** 返回参数说明：
***/
static void fb_rect_draw(int x, int y, int w, int h, char r, char g, char b)
{
	unsigned char *addr = fb_adder + y * 1024 * 3 + x * 3;
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			addr[i * 3 + 2] = r;
			addr[i * 3 + 1] = g;
			addr[i * 3] = b;
		}
		addr += 1024 * 3;
	}
}

// static void draw_rect(int x,int y,int w,int h,int color){

// 	unsigned char* dst_addr = fb_adder + y*1024*3 + x*3;
// 	unsigned char r = (color>>16)&0xFF;
// 	unsigned char g = (color>>8)&0xFF;
// 	unsigned char b = (color)&0xFF;
// 	int j,i;
// 	int line_size = w*3;
// 	for( j=0; j < h ; j++){
		
// 		for(i = 0 ; i < line_size ; i+=3){
			
// 			dst_addr[j*1024*3 + i] = r;
// 			dst_addr[j*1024*3 + i+1] = g;
// 			dst_addr[j*1024*3 + i+2] = b;
// 		}
// 	}
// }
// static void draw_progress_bg(void){

//     draw_rect(212, 290, 600, 1, 0x2693FF);
//     draw_rect(811, 290, 1, 20, 0x2693FF);
//     draw_rect(212, 290, 1, 20, 0x2693FF);
//     draw_rect(212, 309, 600, 1, 0x2693FF);
// }
/***
** 日期: 2022-05-18 15:18
** 作者: leo.liu
** 函数作用：初始化矩形框
** 返回参数说明：
***/
static void fb_rect_init(void)
{
	/***** x:394 y:288 w:13 h:704 *****/
	fb_rect_draw(212, 290, 2, 20, 0xFF, 0x5c, 0x26);
	fb_rect_draw(811, 290, 2, 20, 0xFF, 0x5c, 0x26);
	fb_rect_draw(212, 290, 600, 2, 0xFF, 0x5c, 0x26);
	fb_rect_draw(212, 309, 600, 2, 0xFF, 0x5c, 0x26);
}

/***
** 日期: 2022-05-18 16:06
** 作者: leo.liu
** 函数作用：进度条显示
** 返回参数说明：
***/
static void upgrade_fb_progress_display(void)
{
	fb_rect_draw(215, 293, kernel_upgrade_total ? (kernel_upgrade_count * 594 / kernel_upgrade_total) : 594,15, 0xFF, 0x5c, 0x26);
}

/***
** 日期: 2022-05-18 14:51
** 作者: leo.liu
** 函数作用：检测板级文件升级
** 返回参数说明：
***/
static unsigned int platform_mask = 0x00;
static void detect_platform_check(void)
{
	// if(access(SD_PATH "platform/", F_OK) == 0)
    // return ;
	platform_mask = 0x00;
	if (access(FIRMWARE_PATH UBOOT, F_OK) == 0)
	{
	     kernel_upgrade_total++;
	     platform_mask |= 0x01;
	     printf("find:%s \n",UBOOT);
	}
	if (access(FIRMWARE_PATH ENV, F_OK) == 0)
	{
		kernel_upgrade_total += 2;
		platform_mask |= 0x02;
		printf("find:%s \n",ENV);
	}
	if (access(FIRMWARE_PATH DTB, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x04;
		printf("find:%s \n",DTB);
	}
	if (access(FIRMWARE_PATH KERNEL, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x08;
		printf("find:%s \n",KERNEL);
	}
	if (access(FIRMWARE_PATH LOGO, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x10;
		printf("find:%s \n",LOGO);
	}
	if (access(FIRMWARE_PATH ROOTFS, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x20;
		printf("find:%s \n",ROOTFS);
	}
	if (access(FIRMWARE_PATH CONFIG, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x40;
		printf("find:%s \n",CONFIG);
	}
	if (access(FIRMWARE_PATH APP, F_OK) == 0)
	{
		kernel_upgrade_total++;
		platform_mask |= 0x80;
		printf("find:%s \n",APP);
	}
}

/***
** 日期: 2022-05-18 15:52
** 作者: leo.liu
** 函数作用：获取文件大小
** 返回参数说明：
***/
static int upgrade_file_size_get(const char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		return -1;
	}
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	return size;
}

/***
** 日期: 2022-05-18 15:52
** 作者: leo.liu
** 函数作用：升级u-boot
** 返回参数说明：
***/
#if 1
 static bool upgrade_u_boot_firmware(void)
{
    int size = upgrade_file_size_get(FIRMWARE_PATH UBOOT);
    if (size < 0 || size > UBOOT_SIZE)
    {
        return false;
    }
    /* mtd0: 00037000 00001000 "UBOOT" */
    system("mtd_debug erase /dev/mtd0 0 0x00037000");

    char cmd[128] = {0};
    sprintf(cmd, "mtd_debug write /dev/mtd0 0 %d %s", size, FIRMWARE_PATH UBOOT);
    printf(" %s \n",cmd);
    system(cmd);
    kernel_upgrade_count++;
    return true;
}
#endif

static bool upgrade_nor_img_firmware(void)
{
	/*
    mtd1: 00001000 00001000 "ENV"
    mtd2: 00001000 00001000 "ENVBK"
     */
	int size = upgrade_file_size_get(FIRMWARE_PATH ENV);
	if ((size < 0) || (size > ENV_SIZE))
	{
		return false;
	}

	system("mtd_debug erase /dev/mtd1 0 0x00001000");
	system("mtd_debug erase /dev/mtd2 0 0x00001000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd1 0 %d %s", size, FIRMWARE_PATH ENV);
	printf(" %s \n", cmd);
	system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "mtd_debug write /dev/mtd2 0 %d %s", size, FIRMWARE_PATH ENV);
	printf(" %s \n", cmd);
	system(cmd);

	kernel_upgrade_count++;
	return true;
}

static bool upgrade_dtb_firmware(void)
{
	/* mtd3: 00010000 00001000 "DTB"" */
	int size = upgrade_file_size_get(FIRMWARE_PATH DTB);
	if ((size < 0) || (size > 0x00010000))
	{
		return false;
	}

	system("mtd_debug erase /dev/mtd3 0 0x00010000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd3 0 %d %s", size, FIRMWARE_PATH DTB);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}

static bool upgrade_uimage_firmware(void)
{
	int size = upgrade_file_size_get(FIRMWARE_PATH KERNEL);
	if ((size < 0) || (size > 0x0019c000))
	{
		return false;
	}
	/* mtd4: 00190000 00001000 "KERNEL" */
	system("mtd_debug erase /dev/mtd4 0 0x0019c000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd4 0 %d %s", size, FIRMWARE_PATH KERNEL);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}

static bool upgrade_logo_firmware(void)
{
	int size = upgrade_file_size_get(FIRMWARE_PATH LOGO);
	if ((size < 0) || (size > 0x000019000))
	{
		return false;
	}
	/* mtd5: 00025000 00001000 "LOGO" */
	system("mtd_debug erase /dev/mtd5 0 0x000019000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd5 0 %d %s", size, FIRMWARE_PATH LOGO);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}

static bool upgrade_rootfs_firmware(void)
{
	int size = upgrade_file_size_get(FIRMWARE_PATH ROOTFS);
	if ((size < 0) || (size > 0x263000))
	{
		return false;
	}
	/* mtd6: 001b6000 00001000 "ROOTFS" */
	system("mtd_debug erase /dev/mtd6 0 0x263000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd6 0 %d %s", size, FIRMWARE_PATH ROOTFS);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}
static bool upgrade_config_firmware(void)
{
	int size = upgrade_file_size_get(FIRMWARE_PATH CONFIG);
	if ((size < 0) || (size > 0x8ca000))
	{
		return false;
	}
	/*mtd7: 0097e000 00001000 "CONFIG" */
	system("mtd_debug erase /dev/mtd7 0 0x8ca000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd7 0 %d %s", size, FIRMWARE_PATH CONFIG);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}

static bool upgrade_usr_firmware(void)
{
	int size = upgrade_file_size_get(FIRMWARE_PATH APP);
	if ((size < 0) || (size > 0x2d5000))
	{
		return false;
	}
	/*mtd8: 002ce000 00001000 "APP"*/
	system("mtd_debug erase /dev/mtd8 0 0x2d5000");

	char cmd[128] = {0};
	sprintf(cmd, "mtd_debug write /dev/mtd8 0 %d %s", size, FIRMWARE_PATH APP);
	printf(" %s \n", cmd);
	system(cmd);
	kernel_upgrade_count++;
	return true;
}
/***
** 日期: 2022-05-18 15:04
** 作者: leo.liu
** 函数作用：更新板机文件
** 返回参数说明：
***/
static void upgrade_platform_firmware(void)
{
	 if ((platform_mask & 0x01) && (access(FIRMWARE_PATH UBOOT, F_OK)==0))
	 {
	     upgrade_u_boot_firmware();
	     upgrade_fb_progress_display();
	 }
	if ((platform_mask & 0x02) && (access(FIRMWARE_PATH ENV, F_OK) == 0))
	{
		upgrade_nor_img_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x04) && (access(FIRMWARE_PATH DTB, F_OK) == 0))
	{
		upgrade_dtb_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x08) && (access(FIRMWARE_PATH KERNEL, F_OK) == 0))
	{
		upgrade_uimage_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x10) && (access(FIRMWARE_PATH LOGO, F_OK) == 0))
	{
		upgrade_logo_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x20) && (access(FIRMWARE_PATH ROOTFS, F_OK) == 0))
	{
		upgrade_rootfs_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x40) && (access(FIRMWARE_PATH CONFIG, F_OK) == 0))
	{
		upgrade_config_firmware();
		upgrade_fb_progress_display();
	}
	if ((platform_mask & 0x80) && (access(FIRMWARE_PATH APP, F_OK) == 0))
	{
		upgrade_usr_firmware();
		upgrade_fb_progress_display();
	}
}

/***
** 日期: 2022-05-18 14:54
** 作者: leo.liu
** 函数作用：铃声文件检测
** 返回参数说明：
***/
#ifdef APP_FILE_UPGRADE
static unsigned int rings_mask = 0x00;
static void detect_rings_check(void)
{
	rings_mask = 0x00;
	if (access(FIRMWARE_PATH "rings/1.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x01;
		printf("find:1.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/2.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x02;
		printf("find:2.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/3.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x04;
		printf("find:3.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/4.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x08;
		printf("find:4.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/5.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x10;
		printf("find:5.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/6.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x20;
		printf("find:6.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/7.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x40;
		printf("find:7.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/8.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x80;
		printf("find:8.mp3 \n");
	}
	if (access(FIRMWARE_PATH "rings/10.mp3", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rings_mask |= 0x100;
		printf("find:10.mp3 \n");
	}
}

static void upgrade_rings_firmware(void)
{
	if (access("/etc/config/run", F_OK) != 0)
	{
		system("mkdir /etc/config/run");
	}
	if (access("/etc/config/run/rings", F_OK) != 0)
	{
		system("mkdir /etc/config/run/rings");
	}
	if ((rings_mask & 0x01) && (access(FIRMWARE_PATH "rings/1.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/1.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/1.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/1.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x02) && (access(FIRMWARE_PATH "rings/2.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/2.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/2.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/2.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x04) && (access(FIRMWARE_PATH "rings/3.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/3.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/3.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/3.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x08) && (access(FIRMWARE_PATH "rings/4.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/4.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/4.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/4.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x10) && (access(FIRMWARE_PATH "rings/5.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/5.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/5.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/5.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x10) && (access(FIRMWARE_PATH "rings/6.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/6.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/6.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/6.mp3  /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x20) && (access(FIRMWARE_PATH "rings/7.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/7.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/7.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/7.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x40) && (access(FIRMWARE_PATH "rings/8.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/8.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/8.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/8.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((rings_mask & 0x80) && (access(FIRMWARE_PATH "rings/10.mp3", F_OK) == 0))
	{
		if (access("/etc/config/run/rings/10.mp3", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rings/10.mp3");
		}
		system("cp " FIRMWARE_PATH "rings/10.mp3 /etc/config/run/rings/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
}

/***
** 日期: 2022-05-18 14:57
** 作者: leo.liu
** 函数作用：更新壁纸检测
** 返回参数说明：
***/
static unsigned int wallpaper_mask = 0x00;
static void detect_wallpaper_check(void)
{
	if (access(FIRMWARE_PATH "wallpaper/frame_1.jpg", F_OK) == 0)
	{
		kernel_upgrade_total++;
		wallpaper_mask |= 0x01;
		printf("find:frame_1.jpg \n");
	}
	if (access(FIRMWARE_PATH "wallpaper/frame_2.jpg", F_OK) == 0)
	{
		kernel_upgrade_total++;
		wallpaper_mask |= 0x02;
		printf("find:frame_2.jpg \n");
	}
}
static void upgrade_wallpaper_firmware(void)
{
	if (access("/etc/config/run", F_OK) != 0)
	{
		system("mkdir /etc/config/run");
	}
	if (access("/etc/config/run/wallpaper", F_OK) != 0)
	{
		system("mkdir /etc/config/run/wallpaper");
	}
	if ((wallpaper_mask & 0x01) && (access(FIRMWARE_PATH "wallpaper/frame_1.jpg", F_OK) == 0))
	{
		if (access("/etc/config/run/wallpaper/frame_1.jpg", F_OK) == 0)
		{
			system("rm -f /etc/config/run/wallpaper/frame_1.jpg");
		}
		system("cp " FIRMWARE_PATH "wallpaper/frame_1.jpg /etc/config/run/wallpaper/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
	if ((wallpaper_mask & 0x02) && (access(FIRMWARE_PATH "wallpaper/frame_2.jpg", F_OK) == 0))
	{
		if (access("/etc/config/run/wallpaper/frame_2.jpg", F_OK) == 0)
		{
			system("rm -f /etc/config/run/wallpaper/frame_2.jpg");
		}
		system("cp " FIRMWARE_PATH "wallpaper/frame_2.jpg /etc/config/run/wallpaper/");
		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
}

/***
** 日期: 2022-05-18 14:59
** 作者: leo.liu
** 函数作用：检测app更新
** 返回参数说明：
***/
static unsigned int app_mask = 0x00;
static void detect_app_check(void)
{
	if (access(FIRMWARE_PATH APP_SOFTWARE, F_OK) == 0)
	{
		kernel_upgrade_total++;
		app_mask |= 0x01;
		printf("find:%s \n",APP_SOFTWARE);
	}
}
static void upgrade_app_firmware(void)
{
	if ((app_mask & 0x01) && (access(FIRMWARE_PATH APP_SOFTWARE, F_OK) == 0))
	{
		if (access("/etc/config/"APP_SOFTWARE, F_OK) == 0)
		{
			system("rm -f /etc/config"APP_SOFTWARE);
		}
		system("cp " FIRMWARE_PATH APP_SOFTWARE "/etc/config/run/");

		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
}
/***
** 日期: 2022-05-18 15:01
** 作者: leo.liu
** 函数作用：探测ui资源
** 返回参数说明：
***/
static unsigned int rom_bin_mask = 0x00;
static void detect_rom_check(void)
{
	if (access(FIRMWARE_PATH "rom.bin", F_OK) == 0)
	{
		kernel_upgrade_total++;
		rom_bin_mask |= 0x01;
		printf("find:rom.bin \n");
	}
}
static void upgrade_rom_bin_firmware(void)
{
	if ((rom_bin_mask & 0x01) && (access(FIRMWARE_PATH "rom.bin", F_OK) == 0))
	{
		if (access("/etc/config/run/rom.bin", F_OK) == 0)
		{
			system("rm -f /etc/config/run/rom.bin");
		}
		system("cp " FIRMWARE_PATH "rom.bin /etc/config/run/");

		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
}

/***
** 日期: 2022-05-18 15:02
** 作者: leo.liu
** 函数作用：检测字库更新
** 返回参数说明：
***/
static unsigned int font_mask = 0x00;
static void detect_font_check(void)
{
	if (access(FIRMWARE_PATH "sat_leo.ttf", F_OK) == 0)
	{
		kernel_upgrade_total++;
		font_mask |= 0x01;
		printf("find:sat_leo.ttf \n");
	}
}

static void upgrade_font_firmware(void)
{
	if ((font_mask & 0x01) && (access(FIRMWARE_PATH "sat_leo.ttf", F_OK) == 0))
	{
		if (access("/etc/config/run/sat_leo.ttf", F_OK) == 0)
		{
			system("rm -f /etc/config/run/sat_leo.ttf");
		}
		system("cp " FIRMWARE_PATH "sat_leo.ttf /etc/config/run/");

		kernel_upgrade_count++;
		upgrade_fb_progress_display();
	}
}
#endif
/***
** 日期: 2022-05-18 15:03
** 作者: leo.liu
** 函数作用：更新固件
** 返回参数说明：
***/
static bool upgrade_firmware(void)
{
    if (platform_mask != 0)
    {

        upgrade_platform_firmware();
        system("sync");
        // system("reboot ");
		return true;
        // while (1)
        //     ;
    }
	#ifdef APP_FILE_UPGRADE
	if (rings_mask != 0)
	{
		upgrade_rings_firmware();
	}
	if (wallpaper_mask != 0)
	{
		upgrade_wallpaper_firmware();
	}
	if (app_mask != 0)
	{
		upgrade_app_firmware();
	}
	if (rom_bin_mask != 0)
	{
		upgrade_rom_bin_firmware();
	}
	if (font_mask != 0)
	{
		upgrade_font_firmware();
	}
	system("rm -rf " FIRMWARE_PATH);
	system("sync");
	#endif
	return true;
}
#if 0
/***
**   日期:2022-06-10 15:27:35
**   作者: leo.liu
**   函数作用：清空tuya数据
**   参数说明:
***/
static void tuya_user_data_clear(void)
{
	system("rm -rf " TUYA_UUID_AND_KEY_CONF_PATH);
	system("mkdir " TUYA_UUID_AND_KEY_CONF_PATH);
	system("cp " FIRMWARE_PATH "2022-05-27.conf " TUYA_UUID_AND_KEY_CONF_PATH);
	system("sync");
}
#endif
void upgrade_check_firmware(void)
{
	printf("find frimware start \n");

	/***** 检测板级文件更新 *****/
	detect_platform_check();
	if (platform_mask)
	{
		// system("mkdir "SD_PATH "platform/");//此文件用来判断是否还需要再次升级内核
		// system("mkdir -p "SD_PATH"tuya_backup/tuya_key");
		// system("cp -r /etc/config/tuya_key/2* "SD_PATH "tuya_backup/tuya_key/");//升级内核时备份涂ID，需要重新连网
		goto platform;
	}
	#ifdef APP_FILE_UPGRADE
	/***** 检测铃声升级*****/
	detect_rings_check();

	/***** 检测壁纸更新 *****/
	detect_wallpaper_check();

	/***** 检测执行文件 *****/
	detect_app_check();

	/***** 检测ui资源 *****/
	detect_rom_check();

	/***** 探测字库 *****/
	detect_font_check();
	#endif
platform:
	if (kernel_upgrade_total > 0)
	{
		fb_init();
		fb_rect_init();
		// while ((1))
		// {
		// 	/* code */
		// }
		// draw_progress_bg();
		if(upgrade_firmware())
		{
			printf("UPGRADE FINISH \n");
			
		}
		// if(access(SD_PATH "platform/", F_OK) == 0)
        // {
        //     system("rm -rf " SD_PATH "platform/");
        // }
		// if(access(SD_PATH "tuya_backup/",F_OK) ==0 )
		// {
		// 	system("cp -r "SD_PATH"tuya_backup/*  /etc/config/");
		// 	system("rm -rf " SD_PATH "tuya_backup/");
		// }
		while (1)
		{
			usleep(1000 * 1000);
			fb_rect_draw(0, 0, 1024, 600, 0xFF, 0, 0);
			usleep(1000 * 1000);
			fb_rect_draw(0, 0, 1024, 600, 0xFF, 0xFF, 0xFF);
		}
	}
}