#include "lv_jpg_decode.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "jpeglib.h"
#include <setjmp.h>
#include "../include/anyka/ak_common_graphics.h"
#include "../include/anyka/ak_mem.h"
#include "../include/anyka/ak_tde.h"

struct my_error_mgr
{
	struct jpeg_error_mgr pub; /* "public" fields */

	jmp_buf setjmp_buffer; /* for return to caller */
};
typedef struct my_error_mgr *my_error_ptr;

static void my_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message)(cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

/***
**   日期:2022-07-01 13:50:29
**   作者: leo.liu
**   函数作用：获取解码的数据
**   参数说明:
***/
bool lv_jpg_decode_data(const char *file, rom_bin_info *info, int dst_w, int dst_h)
{
	FILE *jpeg_fd = fopen(file, "rb");
	if (jpeg_fd == NULL)
	{
		printf("jpg decode :open %s failed \n", file);
		return false;
	}
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;

	/* Step 1: allocate and initialize JPEG decompression object */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(jpeg_fd);
		return false;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);
	/* Step 2: specify data source (eg, a file) */
	jpeg_stdio_src(&cinfo, jpeg_fd);
	/* Step 3: read file parameters with jpeg_read_header() */
	(void)jpeg_read_header(&cinfo, TRUE);

	/* Step 4: set parameters for decompression */
	cinfo.out_color_space = JCS_RGB;
	if ((dst_w * dst_h) < (cinfo.image_width * cinfo.image_height))
	{
		if (((abs(dst_w - cinfo.image_width)) < (abs(dst_h - cinfo.image_height))) && (dst_h < cinfo.image_height))
		{
			cinfo.scale_num = dst_h;
			cinfo.scale_denom = cinfo.image_height;
		}
		else
		{
			cinfo.scale_num = dst_h;
			cinfo.scale_denom = cinfo.image_width;
		}
	}
	/* Step 5: Start decompressor */
	(void)jpeg_start_decompress(&cinfo);
	// printf("image_width = %d out_wdith=%d\n", cinfo.image_width, cinfo.output_width);
	// printf("image_height = %d out_hegiht=%d\n", cinfo.image_height, cinfo.output_height);
	// printf("num_components = %d\n", cinfo.num_components);

	
	/* JSAMPLEs per row in output buffer */
	
	int row_stride = cinfo.output_width * cinfo.output_components;
	unsigned char *jpeg_raw_data = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_GUI, cinfo.output_height * row_stride);
	/* Make a one-row-high sample array that will go away when done with image */
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
	
	while (cinfo.output_scanline < cinfo.output_height)
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could ask for
		 * more than one scanline at a time if that's more convenient.
		 */
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);
		// printf("====%s:%d  @@@@@@@@@@@@@@@@@ %d h:%d  line:%d\n",__func__,__LINE__,row_stride,cinfo.output_height,cinfo.output_scanline);
		memcpy(&jpeg_raw_data[(cinfo.output_scanline-1) * row_stride], buffer[0], row_stride);
		/* Assume put_scanline_someplace wants a pointer and sample count. */
	}
	
	struct ak_tde_layer src, dst;
	src.format_param = GP_FORMAT_RGB888;
	src.pos_width = src.width = cinfo.output_width;
	src.pos_height = src.height = cinfo.output_height;
	src.pos_left = src.pos_top = 0;
	ak_mem_dma_vaddr2paddr(jpeg_raw_data, (unsigned long *)&src.phyaddr);
	
#if LV_COLOR_DEPTH == 32
	dst.format_param = GP_FORMAT_BGR888;
#elif LV_COLOR_DEPTH == 16
	dst.format_param = GP_FORMAT_BGR565;
#endif
	dst.pos_width = dst.width = dst_w;
	dst.pos_height = dst.height = dst_h;
	dst.pos_left = dst.pos_top = 0;
	
	unsigned char *data = (unsigned char *)info->offset;
	ak_mem_dma_vaddr2paddr(data, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_scale(&src, &dst);
	
	ak_mem_dma_free(jpeg_raw_data);
#if LV_COLOR_DEPTH == 32
	int len = dst_w * dst_h;
	unsigned char *dst_data = data + dst_w * dst_h * 4 - 1;
	unsigned char *src_data = data + dst_w * dst_h * 3 - 1;
	for (int i = 0; i < len; i++)
	{

		*dst_data = 0xFF;
		dst_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
		*dst_data = *src_data;
		dst_data--;
		src_data--;
	}
#endif

	/* Step 7: Finish decompression */
	(void)jpeg_finish_decompress(&cinfo);
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);
	
	fclose(jpeg_fd);
	return true;
}