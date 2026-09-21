/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"

#include "../include/anyka/ak_common.h"
#include "../include/anyka/ak_common_graphics.h"
#include "../include/anyka/ak_tde.h"
#include "../include/anyka/ak_mem.h"
#include "../include/anyka/ak_vdec.h"

#include <linux/fb.h>

#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include "unistd.h"
#include "../include/anyka/ak_thread.h"

#define FBDEV_PATH "/dev/fb0"

#define LCD_FORMAT_24

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

#if LV_USE_GPU
// static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_blend(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, const lv_color_t *src_buf, const lv_area_t *src_area, const lv_area_t *dst_area, lv_opa_t opa, bool a_color);

static void gpu_fill(lv_disp_drv_t *disp_drv, lv_color_t *dest_buf, lv_coord_t dest_width,
					 const lv_area_t *fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;

static unsigned char *fbp = 0;
static unsigned long fbp_phyaddres = 0;
static long int screensize = 0;
static int fbfd = 0;

#define FB_BUFFER_A 0
#define FB_BUFFER_B 1
#define FB_BUFFER_GET ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)
#define FB_BUFFER_SET ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);
#define FB_BUFFER vinfo.reserved[0]

static ak_mutex_t gui_buffer_mutex;
static unsigned char *gui_buffer = NULL;
static unsigned long gui_phyaddres = 0;

static sem_t lv_fb_sem_t;
static void *platform_refresh_screen_task(void *arg);
static unsigned char *lv_gui_addres = NULL;
void lv_port_disp_init(void)
{
	disp_init();

	static lv_disp_buf_t draw_buf_dsc_3;
	lv_gui_addres = (unsigned char *)ak_mem_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	lv_disp_buf_init(&draw_buf_dsc_3, lv_gui_addres, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);

	disp_drv.hor_res = LV_HOR_RES_MAX;
	disp_drv.ver_res = LV_VER_RES_MAX;

	disp_drv.flush_cb = disp_flush;

	/*Set a display buffer*/
	disp_drv.buffer = &draw_buf_dsc_3;

#if LV_USE_GPU
	disp_drv.gpu_blend_cb = gpu_blend;

	/*Fill a memory array with a color*/
	disp_drv.gpu_fill_cb = gpu_fill;
#endif
	lv_disp_drv_register(&disp_drv);

	ak_pthread_t thread_t;
	ak_thread_create(&thread_t, platform_refresh_screen_task, NULL, 200 * 1024, -1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
	if (access(FBDEV_PATH, F_OK) != 0)
	{
		printf("%s ==============================>>>%d\n\r", __func__, __LINE__);
		system("rmmod /usr/modules/ak_fb.ko");
		system("insmod /usr/modules/ak_fb.ko");
		system("insmod /usr/modules/ak_gui.ko");
	}

	sdk_run_config config = {0};
	config.mem_trace_flag = SDK_RUN_NORMAL;
	ak_sdk_init(&config);
	ak_tde_open();

	/*You code here*/
	fbfd = open(FBDEV_PATH, O_RDWR | O_EXCL);
	if (fbfd == -1)
	{

		perror("Error: cannot open framebuffer device");
		return;
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
	{
		perror("Error reading fixed information");
		return;
	}

	// Get variable screen information
	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
	{
		perror("Error reading variable information");
		return;
	}

	vinfo.xres = LV_HOR_RES_MAX;
	vinfo.yres = LV_VER_RES_MAX;
#ifdef LCD_FORMAT_24
	vinfo.bits_per_pixel = 24;
	vinfo.red.offset = 16;
	vinfo.red.length = 8;
	vinfo.green.offset = 8;
	vinfo.green.length = 8;
	vinfo.blue.offset = 0;
	vinfo.blue.length = 8;

#else
	vinfo.bits_per_pixel = 16;
	vinfo.red.offset = 11;
	vinfo.red.length = 5;
	vinfo.green.offset = 5;
	vinfo.green.length = 6;
	vinfo.blue.offset = 0;
	vinfo.blue.length = 5;
#endif
	ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo);

	int bpp = vinfo.bits_per_pixel;
	screensize = vinfo.xres * vinfo.yres * bpp / 8;

	printf("phy:%lu ,screensize:%ld smem_len:%u\n", fbp_phyaddres + screensize, screensize, finfo.smem_len);
	extern void *osal_fb_mmap_viraddr(int fb_len, int fb_fd);
	fbp = (unsigned char *)osal_fb_mmap_viraddr(screensize * 2, fbfd);

	fbp_phyaddres = finfo.smem_start;

	printf("fb buffer 1:%p,2:%p ,phy:%lu ,2:%lu smem_len:%u\n", fbp, fbp + screensize, fbp_phyaddres, fbp_phyaddres + screensize, finfo.smem_len);

	gui_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 4);
	ak_mem_dma_vaddr2paddr(gui_buffer, (unsigned long *)&gui_phyaddres);
	ak_thread_mutex_init(&gui_buffer_mutex, NULL);

	FB_BUFFER = FB_BUFFER_A;
	struct ak_tde_layer layer;
#ifdef LCD_FORMAT_24
	layer.format_param = GP_FORMAT_RGB888;
#else
	layer.format_param = GP_FORMAT_RGB565;
#endif
	layer.width = LV_HOR_RES_MAX; // screen_vector.width;
	layer.height = LV_VER_RES_MAX;
	layer.pos_left = 0;
	layer.pos_top = 0;
	layer.pos_width = LV_HOR_RES_MAX;
	layer.pos_height = LV_VER_RES_MAX;
	layer.phyaddr = FB_BUFFER == FB_BUFFER_B ? (fbp_phyaddres + screensize) : fbp_phyaddres;
	ak_tde_opt_fillrect(&layer, 0x00);
	FB_BUFFER_SET;
	printf("The framebuffer device was mapped to memory successfully.:%p\n", gui_buffer);
}

static unsigned char *system_bg_addres = NULL;
static unsigned char *system_bg_temp_addres = NULL;
static unsigned long system_bg_phyaddres = 0;

static bool fb_dst_tde_layer_get(struct ak_tde_layer *dst, const lv_area_t *area_t)
{
#ifdef LCD_FORMAT_24
	dst->format_param = GP_FORMAT_RGB888;
#else
	dst->format_param = GP_FORMAT_RGB565;
#endif
	dst->width = LV_HOR_RES_MAX; // screen_vector.width;
	dst->height = LV_VER_RES_MAX;
	dst->pos_left = area_t->x1;
	dst->pos_top = area_t->y1;
	dst->pos_width = lv_area_get_width(area_t) - 1;
	dst->pos_height = lv_area_get_height(area_t) - 1;
	dst->phyaddr = FB_BUFFER == FB_BUFFER_B ? (fbp_phyaddres + screensize) : fbp_phyaddres;
	return true;
}

static bool bg_tde_layer_get(struct ak_tde_layer *dst)
{
	dst->format_param = GP_FORMAT_YUV420SP;
	dst->width = LV_HOR_RES_MAX;
	dst->height = LV_VER_RES_MAX;
	dst->pos_left = 0;
	dst->pos_top = 0;
	dst->pos_width = LV_HOR_RES_MAX;
	dst->pos_height = LV_VER_RES_MAX;
	if (system_bg_addres == NULL)
	{
		system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
		ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	}
	dst->phyaddr = system_bg_phyaddres;
	return true;
}

static bool gui_tde_layer_get(struct ak_tde_layer *dst, const lv_area_t *area_t)
{
	dst->format_param = GP_FORMAT_ARGB8888;
	dst->width = LV_HOR_RES_MAX; // screen_vector.width;
	dst->height = LV_VER_RES_MAX;
	dst->pos_left = area_t->x1;
	dst->pos_top = area_t->y1;
	dst->pos_width = lv_area_get_width(area_t) - 1;
	dst->pos_height = lv_area_get_height(area_t) - 1;
	dst->phyaddr = gui_phyaddres;
	return true;
}

static void system_bg_decode_data_copy(struct ak_vdec_frame *frame)
{
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = frame->frame_obj.data.pitch_width;
	src.height = frame->frame_obj.data.pitch_height;
	src.pos_top = src.pos_left = 0;
	src.pos_width = frame->width;
	src.pos_height = frame->height;
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);

	bg_tde_layer_get(&dst);
	if ((frame->width == LV_HOR_RES_MAX) && (frame->height == LV_VER_RES_MAX))
	{
		ak_tde_opt_blit(&src, &dst);
	}
	else
	{
		ak_tde_opt_scale(&src, &dst);
	}
}

#if 0
static void *bg_load_close_task(void *arg)
{
	int *handle_id = (int *)arg;
	ak_vdec_close(*handle_id);
	*handle_id = -1;
	ak_thread_exit();
	return NULL;
}
#endif

static void system_bg_resource_load(rom_bin_info *info)
{
	if (info == NULL)
	{
		return;
	}
	// unsigned long long os_get_ms();
	// unsigned long long x = os_get_ms();
	extern const unsigned char *get_rom_bin_base(void);
	static int handle_id = -1;
	struct ak_vdec_param param;
	memset(&param, 0, sizeof(struct ak_vdec_param));
	param.vdec_type = MJPEG_ENC_TYPE;
	param.output_type = AK_YUV420SP;
	param.sc_width = LV_HOR_RES_MAX;
	param.sc_height = LV_VER_RES_MAX;
	param.frame_buf_num = 0;

	// x = os_get_ms();
	if (handle_id == (-1))
	{
		ak_vdec_open(&param, &handle_id);
	}
	ak_vdec_clear_buff(handle_id);

	const unsigned char *pdata = get_rom_bin_base() + info->offset;
	int dec_len = 0;
	int read_len = info->size;
	int send_len = 0;
	printf("%s================!!!!!!!!!!!!!!!!!!:%d\n\n", __func__, read_len);
	while (read_len > 0)
	{
		ak_vdec_send_stream(handle_id, &pdata[send_len], read_len, NONBLOCK, &dec_len);
		read_len -= dec_len;
		send_len += dec_len;
	}

	struct ak_vdec_frame frame = {0};

	int timeout = 500;
	while (timeout--)
	{
		memset(&frame, 0, sizeof(frame));
		if (ak_vdec_get_frame(handle_id, &frame) == 0)
		{
			system_bg_decode_data_copy(&frame);
			ak_vdec_release_frame(handle_id, &frame);
			break;
		}
		ak_sleep_ms(1);
	}

	// ak_pthread_t thread_id;
	// ak_thread_create(&thread_id, bg_load_close_task, &handle_id, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	// ak_thread_detach(thread_id);
}

static bool system_bg_enable = false;
void system_bg_enable_set(bool en)
{
	system_bg_enable = en;
}
bool system_bg_loading(rom_bin_info *info, bool extern_src)
{
	if (extern_src)
	{
		lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_COVER);
		lv_disp_set_bg_image(lv_disp_get_default(), info);
	}
	else
	{
		lv_disp_set_bg_opa(lv_disp_get_default(), LV_OPA_TRANSP);
		system_bg_resource_load(info);
	}

	system_bg_enable = true;
	return true;
}

unsigned long system_bg_phyaddres_get(void)
{
	if (system_bg_addres == NULL)
	{
		system_bg_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
		ak_mem_dma_vaddr2paddr(system_bg_addres, (unsigned long *)&system_bg_phyaddres);
	}

	return system_bg_phyaddres;
}

bool system_bg_data_backup(void)
{
	if (system_bg_addres == NULL)
	{
		return false;
	}
	if (system_bg_temp_addres == NULL)
	{
		system_bg_temp_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
	}
	memcpy(system_bg_temp_addres, system_bg_addres, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
	/* 使用dma内存copy，会出现颜色失真*/
#if 0
	struct ak_tde_layer src,dst;
	bg_tde_layer_get(&src);

	dst = src;
	if(system_bg_temp_addres == NULL)
	{
		system_bg_temp_addres = (unsigned char*)ak_mem_dma_alloc(MODULE_ID_VO,LV_HOR_RES_MAX*LV_VER_RES_MAX*3/2);
	}
	ak_mem_dma_vaddr2paddr(system_bg_temp_addres, (unsigned long*)&dst.phyaddr);
	ak_tde_opt_blit(&src, &dst);
#endif
#if 0
	int fd = open("/mnt/yuvsp",O_WRONLY|O_CREAT);
	write(fd,system_bg_temp_addres,LV_HOR_RES_MAX*LV_VER_RES_MAX*3/2);
	close(fd);
#endif
	return true;
}

bool system_bg_data_recovery(void)
{
	if (system_bg_temp_addres == NULL)
	{
		return false;
	}

	memcpy(system_bg_addres, system_bg_temp_addres, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);

	/* 使用dma内存copy，会出现颜色失真*/
#if 0
	struct ak_tde_layer src,dst;
	bg_tde_layer_get(&dst);

	src = dst;
	ak_mem_dma_vaddr2paddr(system_bg_temp_addres, (unsigned long*)&src.phyaddr);
	ak_tde_opt_blit(&src, &dst);
#endif
	ak_mem_dma_free(system_bg_temp_addres);
	system_bg_temp_addres = NULL;
	return true;
}

bool system_bg_fill_color(unsigned int color, int x, int y, int w, int h)
{
	if (system_bg_addres == NULL)
	{
		return false;
	}

	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB565;
	src.pos_width = src.width = w;
	src.pos_height = src.height = h;
	src.pos_left = src.pos_top = 0;
	unsigned char *addres = ak_mem_dma_alloc(MODULE_ID_VO, w * h * 2);
	ak_mem_dma_vaddr2paddr(addres, (unsigned long *)&src.phyaddr);
	ak_tde_opt_fillrect(&src, color);

	bg_tde_layer_get(&dst);
	dst.pos_left = x;
	dst.pos_top = y;
	dst.pos_width = w;
	dst.pos_height = h;
	ak_tde_opt_blit(&src, &dst);

	ak_mem_dma_free(addres);
	return true;
}

static bool is_video_mode_enable = false;
static bool is_bg_mode_enable = true;
static bool is_gui_mode_enable = true;
void fb_background_enable(bool en)
{
	if (en)
		lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	else
		lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
}
void fb_video_mode_enable(bool en)
{
	if (is_video_mode_enable == en)
	{
		return;
	}
	printf("%s==========================>%d\n", __func__, en);
	if (en == false)
	{
		if (system_bg_addres == NULL)
		{
			lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
		}
		else
		{
			lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
		}
	}
	else
	{
		lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	}
	// video_raw_clear();
	is_video_mode_enable = en;
}

bool is_fb_video_mode_enable(void)
{
	return is_video_mode_enable;
}

void fb_bg_mode_enable(bool en)
{
	is_bg_mode_enable = en;
}
void fb_gui_mode_enable(bool en)
{
	is_gui_mode_enable = en;
}
#if 0
static bool video_tde_layer_get(struct ak_tde_layer* dst,unsigned char* addres,int width,int height)
{
	dst->format_param = GP_FORMAT_YUV420SP;
	dst->width = width;//screen_vector.width;
	dst->height = height;
	dst->pos_left = 0;
	dst->pos_top = 0;
	dst->pos_width = width;
	dst->pos_height = height;
	ak_mem_dma_vaddr2paddr(addres, (unsigned long *)&dst->phyaddr);
	return true;
}
#endif

static void bg_flush_to_fb(void)
{
	if (is_bg_mode_enable == false)
		return;

	struct ak_tde_layer src, dst;
	lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	bg_tde_layer_get(&src);
	// static bool first = true;
	if (is_video_mode_enable == true)
	{
		extern unsigned char *video_raw_lcd_get(unsigned long long *timestamp);
		ak_mem_dma_vaddr2paddr(video_raw_lcd_get(NULL), (unsigned long *)&src.phyaddr);
		src.format_param = GP_FORMAT_YUV420P;
		// if (first == true)
		// {
		// 	first = false;
		// 	extern void video_raw_clear(void);
		// 	video_raw_clear();
		// }
	}
	// else if (first == false)
	// {
	// 	first = true;
	// }
	fb_dst_tde_layer_get(&dst, &area);
	ak_tde_opt_format(&src, &dst);
}

static int gui_area_count = 0;
static lv_area_t gui_area_group[32] = {{0}};
void gui_draw_area_add(lv_area_t area_t)
{
	return;
	if (gui_area_count < 32)
	{
		gui_area_group[gui_area_count] = area_t;
		gui_area_count++;
	}
}
void gui_draw_area_set(const lv_area_t *area_t, int count)
{
	// return;
	memcpy(gui_area_group, area_t, count * sizeof(lv_area_t));
	gui_area_count = count;
}
static void gui_flush_to_fb(void)
{
	if (is_gui_mode_enable == false)
		return;

	struct ak_tde_layer src, dst;
	for (int i = 0; i < gui_area_count; i++)
	{
		gui_tde_layer_get(&src, &gui_area_group[i]);
		fb_dst_tde_layer_get(&dst, &gui_area_group[i]);
		// printf("gui_area_group = %d %d %d %d\n",gui_area_group[i].x1,gui_area_group[i].x2,gui_area_group[i].y1,gui_area_group[i].y2);
		// printf("lv_area_get = %d %d\n",lv_area_get_width(&gui_area_group[i]),lv_area_get_height(&gui_area_group[i]));
		// printf("src = %d %d,dst = %d %d\n",src.pos_width,src.pos_height,dst.pos_width,dst.pos_height);
		ak_tde_opt_blit(&src, &dst);
	}
	// gui_area_count = 0;
}
/*
static void fb_flush_to_fb(void)
{
	FB_BUFFER_GET;
	FB_BUFFER = FB_BUFFER == FB_BUFFER_A ? FB_BUFFER_B : FB_BUFFER_A;
	if ((system_bg_enable == true) && (system_bg_addres != NULL))
	{
		bg_flush_to_fb();
	}
	gui_flush_to_fb();
	FB_BUFFER_SET;
}
 */

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	if (area->x2 < 0 || area->y2 < 0 || area->x1 > (int32_t)LV_HOR_RES_MAX || area->y1 > (int32_t)LV_VER_RES_MAX)
	{
		lv_disp_flush_ready(disp_drv);
		return;
	}
	// printf("%s==========%d,%d,%d,%d\n",__func__,area->x1,area->y1,area->x2-area->x1,area->y2-area->y1);
	gui_draw_area_add(*area);
	//	video_main_display_lock();
	/***** 将图片tde biblt到gui layer上面 *****/
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB565;
	src.width = lv_area_get_width(area);
	src.height = lv_area_get_height(area);
	// printf("w%d h%d \n", src.width, src.height);
	if (1) // ((src.width < 18) || (src.height < 18))
	{
		ak_thread_mutex_lock(&gui_buffer_mutex);
		unsigned char *src_addr = lv_gui_addres;
		unsigned char *dst = gui_buffer + area->y1 * LV_HOR_RES_MAX * 4 + area->x1 * 4;
		for (int h = 0; h < src.height; h++)
		{
			memcpy(&dst[h * LV_HOR_RES_MAX * 4], &src_addr[h * src.width * 4], src.width * 4);
		}
		ak_thread_mutex_unlock(&gui_buffer_mutex);
	}
	else
	{
		src.pos_left = 0;
		src.pos_top = 0;
		src.width *= 2;
		src.pos_width = src.width;
		src.pos_height = src.height;
		ak_mem_dma_vaddr2paddr((void *)lv_gui_addres, (unsigned long *)&src.phyaddr);

		// memcpy(&dst, &src, sizeof(struct ak_tde_layer));
		dst.format_param = GP_FORMAT_RGB565;
		dst.width = LV_HOR_RES_MAX * 2;
		dst.height = LV_VER_RES_MAX;
		dst.pos_left = area->x1 * 2;
		dst.pos_top = area->y1;
		dst.pos_width = src.width;
		dst.pos_height = src.height;
		dst.phyaddr = gui_phyaddres;
		ak_tde_opt_blit(&src, &dst);
	}
	//	video_main_display_unlock();

	if (lv_disp_flush_is_last(disp_drv) == true)
	{
		int value;
		sem_getvalue(&lv_fb_sem_t, &value);
		if (value == 0)
		{
			sem_post(&lv_fb_sem_t);
		}
	}
	lv_disp_flush_ready(disp_drv);
}

void gui_raw_clear(void)
{
	ak_thread_mutex_lock(&gui_buffer_mutex);
	// printf("%s ==============================>>>%p\n\r", __func__, gui_buffer);
	memset(gui_buffer, 0x00, 1024 * 600 * 4);
	ak_thread_mutex_unlock(&gui_buffer_mutex);

	// void screen_force_refresh(void);
	// screen_force_refresh();
}

static void *platform_refresh_screen_task(void *arg)
{
	printf("***** refresh screen task create sccess ! *****\n");
	//	int average_frame_time = 0;
	//	unsigned long long timestamp = user_timestamp_get();
	//	int frame_count = 0;

	while (1)
	{
		sem_wait(&lv_fb_sem_t);
		FB_BUFFER_GET;
		FB_BUFFER = FB_BUFFER == FB_BUFFER_A ? FB_BUFFER_B : FB_BUFFER_A;
		bg_flush_to_fb();
		gui_flush_to_fb();
		FB_BUFFER_SET;
		usleep(10 * 1000);
	}

	return NULL;
}

void screen_capture(void)
{
	char rgb_file[32] = {0};
	static int x = 0;
	sprintf(rgb_file, "/mnt/tf/%d.rgb", x++);
	printf("screenshot screen :%d!!!\n", x);
	FILE *fp = fopen(rgb_file, "w");
	if (fp == NULL)
	{
		printf("screenshot screen	fail!!!!   creat rgb file fail.\n");
		return;
	}
	int i = LV_HOR_RES_MAX * LV_VER_RES_MAX * 3;

	fwrite(FB_BUFFER == FB_BUFFER_B ? fbp + screensize : fbp, i, 1, fp);

	fclose(fp);
	system("sync");
}
/***
** 日期: 2022-05-13 10:14
** 作者: leo.liu
** 函数作用：强制刷新
** 返回参数说明：
***/
void screen_force_refresh(void)
{
	sem_post(&lv_fb_sem_t);
}
#endif
