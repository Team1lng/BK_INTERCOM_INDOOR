

#include <stdbool.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>


#define FB_PATH "/dev/fb0"
#define FIRMWARE_PATH "/mnt/.upgrade/tmp/"

static int upgrade_total = 0;
static int upgrade_count = 0;

static int h_size = 0;//水平宽度
static int v_size = 0;//垂直宽度

static unsigned char *fb_adder = NULL;
static struct fb_var_screeninfo var_info;
static struct fb_fix_screeninfo fix_info;
static u_int32_t process_color = 0x00FFFF;


static bool fb_init(void)
{
	system("insmod /usr/modules/ak_fb.ko");
	int fd = open(FB_PATH, O_RDWR);
	if (fd < 0)
	{
		printf("open %s failed \n", FB_PATH);
		exit(0);
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
	printf("X:%d  Y:%d  bbp:%d  \n", var_info.xres, var_info.yres, var_info.bits_per_pixel);
	h_size = var_info.xres;
	v_size = var_info.yres;
	/***** 获取不可变参数 *****/
	ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);

	fb_adder = (unsigned char *)mmap(0, fix_info.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb_adder == NULL)
	{
		printf("can't mmap Framebuffer\n");
		exit(0);
	}

	memset((void *)fb_adder, 0x00, fix_info.smem_len);

	var_info.reserved[0] = 0;

	ioctl(fd, FBIOPUT_VSCREENINFO, &var_info);
	return true;
}


static void fb_rect_draw(int x, int y, int w, int h, char r, char g, char b)
{
	unsigned char *addr = fb_adder + y * h_size * 3 + x * 3;
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			addr[i * 3 + 2] = r;
			addr[i * 3 + 1] = g;
			addr[i * 3] = b;
		}
		addr += h_size * 3;
	}
}

static void fb_rect_init(void)
{
	/***** x:394 y:288 w:13 h:704 *****/

	fb_rect_draw(h_size / 6, v_size / 2 - 7, h_size / 6 * 4, 2, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6, v_size / 2 + 7, h_size / 6 * 4, 2, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6, v_size / 2 - 7, 2, 14, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6 * 5 - 2, v_size / 2 - 7, 2, 14, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
}


static void upgrade_fb_progress_display(void)
{
	fb_rect_draw(h_size / 6 + 3, v_size / 2 - 4, upgrade_total ? (upgrade_count * (h_size / 6 * 4 - 6) / upgrade_total) : h_size / 6 * 4 - 6, 10, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("请输入参数，如：[./upgrade_progress 10 1]\n");
        return 0;
    }

    fb_init();

    process_color = 0x00FFFF;

    // process_color = 0xFF8000;

	fb_rect_init();

    upgrade_total = atoi(argv[1]);

    upgrade_count = atoi(argv[2]);

	// printf("=========>>> total:[%d] count:[%d]\n", upgrade_total, upgrade_count);

	upgrade_fb_progress_display();

	int fd = open("/tmp/proc_value", O_RDONLY);

	int proc_value = 0;

	char buffer[4] = {0};

	while(1)
	{
		usleep(500 * 1000);
		memset(buffer, 0, sizeof(buffer));
		lseek(fd, 0, SEEK_SET);
		int res = read(fd, buffer, sizeof(buffer));
		if (res < 0)
		{
			return 1;
		}
		else if(res == 0)
		{
			continue;
		}
		else
		{
			proc_value = atoi(buffer);
			if(proc_value > upgrade_total || proc_value < upgrade_count)
			{
				continue;
			}
			upgrade_count = proc_value;
		}
		upgrade_fb_progress_display();
	}

    return 0;
}