#include "leo_api.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include "queue.h"
#include "ak_mem.h"
#include "ak_vdec.h"
#include "string.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "ak_thread.h"
#include "../lv_conf.h"
#include "video_decode.h"

#define tde_layer_pos_init(layer, px, py, pw, ph) \
	layer.pos_left = px;                      \
	layer.pos_top = py;                       \
	layer.pos_width = pw;                     \
	layer.pos_height = ph;

#define tde_layer_layer_init(layer, fmt, w, h, px, py, pw, ph) \
	layer.format_param = fmt;                              \
	layer.width = w;                                       \
	layer.height = h;                                      \
	tde_layer_pos_init(layer, px, py, pw, ph);

static unsigned char *video_raw_lcd_buffer = NULL;
static unsigned long long video_raw_lcd_timestmap = 0;

void video_raw_init(void)
{
	if (video_raw_lcd_buffer == NULL)
	{
		video_raw_lcd_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VI, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
	}
	video_raw_clear();
}

static void video_raw_biblt(unsigned char *src_addres, unsigned long phy, int src_width, int src_height, int pixel_width, int pixel_height, enum ak_gp_format src_format,
			    unsigned char *dst_addres, int dst_width, int dst_height, enum ak_gp_format dst_format)
{
	struct ak_tde_layer dst, src;
	src.format_param = src_format;
	src.width = pixel_width;   // src_width;
	src.height = pixel_height; // src_height;
	src.pos_left = 0;
	src.pos_top = 0;
	src.pos_width = src_width;
	src.pos_height = src_height;
	if (phy != 0)
	{
		src.phyaddr = phy;
	}
	else
	{
		ak_mem_dma_vaddr2paddr(src_addres, (unsigned long *)&src.phyaddr);
	}

	dst.format_param = dst_format;
	dst.width = dst_width;
	dst.height = dst_height;
	dst.pos_left = 0;
	dst.pos_top = 0;
	dst.pos_width = dst_width;
	dst.pos_height = dst_height;
	ak_mem_dma_vaddr2paddr(dst_addres, (unsigned long *)&dst.phyaddr);
	if ((src_width != dst_width) || (src_height != dst_height))
	{
		ak_tde_opt_scale(&src, &dst);
	}
	else
	{
		ak_tde_opt_blit(&src, &dst);
	}
}

bool video_raw_lcd_push(unsigned char *addres, unsigned long phy, int width, int height, int pixel_width, int pixel_height, AK_GP_FORMAT format)
{
	video_raw_biblt(addres, phy, width, height, pixel_width, pixel_height, GP_FORMAT_YUV420SP, video_raw_lcd_buffer, LV_HOR_RES_MAX, LV_VER_RES_MAX, GP_FORMAT_YUV420P);
	video_raw_lcd_timestmap = get_sys_ms();
	return true;
}

/***
**   日期:2022-06-28 14:10:44
**   作者: leo.liu
**   函数作用：获取视频帧
**   参数说明:
***/
unsigned char *video_raw_lcd_get(unsigned long long *timestamp)
{
	if (timestamp != NULL)
	{
		unsigned long long pre_timestamp = *timestamp;
		if (video_raw_lcd_timestmap == pre_timestamp)
		{
			return NULL;
		}
		*timestamp = video_raw_lcd_timestmap;
	}
	return video_raw_lcd_buffer;
}
#include <fcntl.h>
#include <string.h>

void video_raw_color_rect(int color,int x,int y,int pos_w,int pos_h)
{
	#if 1
	struct ak_tde_layer src ,dst;
	src.format_param = GP_FORMAT_RGB565;
	src.width = src.pos_width = pos_w;
	src.height = src.pos_height = pos_h;
	src.pos_left = 0;
	src.pos_top = 0;

	unsigned char *addres = ak_mem_dma_alloc(MODULE_ID_VO, pos_w * pos_h * 2);
	ak_mem_dma_vaddr2paddr(addres, (unsigned long *)&src.phyaddr);
	ak_tde_opt_fillrect(&src,color);

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.width = LV_HOR_RES_MAX;
	dst.height = LV_VER_RES_MAX;
	dst.pos_left = 0;
	dst.pos_top = 0;
	dst.pos_width = LV_HOR_RES_MAX;
	dst.pos_height = LV_VER_RES_MAX;
	if (video_raw_lcd_buffer == NULL)
	{
		video_raw_lcd_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2);
	}
	ak_mem_dma_vaddr2paddr(video_raw_lcd_buffer, (unsigned long *)&dst.phyaddr);

	dst.pos_left = x;
	dst.pos_top = y;
	dst.pos_width = pos_w;
	dst.pos_height = pos_h;
	ak_tde_opt_blit(&src, &dst);

	ak_mem_dma_free(addres);
	#else/* 
	struct ak_tde_layer src ;
	src.format_param = GP_FORMAT_RGB565;
	src.width = src.pos_width = 1024;
	src.height = src.pos_height = 300;
	src.pos_left = 0;
	src.pos_top = 0;
	ak_mem_dma_vaddr2paddr(video_raw_lcd_buffer, (unsigned long *)&src.phyaddr);
	ak_tde_opt_fillrect(&src,0x00);

	unsigned char*data =  video_raw_lcd_buffer + 1024*600;
	src.width = src.pos_width = 1024;
	src.height = src.pos_height = 100;
	src.pos_left = 0;
	src.pos_top = 0;
	ak_mem_dma_vaddr2paddr(data, (unsigned long *)&src.phyaddr);
	ak_tde_opt_fillrect(&src,0x8080); */

/* 	int fd = open("/mnt/nfs/boke/yuv.yuv",O_WRONLY|O_CREAT);
	if(fd < 0)
	{
		printf("write open %s fail \n","/mnt/nfs/boke/yuv.rgb");
		return;
	}
	write(fd,video_raw_lcd_buffer,(LV_HOR_RES_MAX * LV_VER_RES_MAX * 3 / 2));
	close(fd);
	system("sync"); */
	#endif
}
void video_raw_clear(void)
{
	// Debug_Lib("%s==================================>\n",__func__);
	memset(video_raw_lcd_buffer,0x00,1024*600);
	memset(video_raw_lcd_buffer+ 1024*600,0x80,1024*300);
}
