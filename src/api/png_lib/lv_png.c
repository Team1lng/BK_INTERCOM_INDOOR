/**
 * @file lv_png.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#include "../../lvgl/src/lv_misc/lv_ll.h"
#include "../../lvgl/src/lv_misc/lv_gc.h"

#else
#include "lvgl/lvgl.h"
#endif

#include "lv_png.h"
#include "lodepng.h"
#include <stdlib.h>
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
static const unsigned char *png_bin_base = NULL;
static unsigned int png_bin_size = 0;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(struct _lv_img_decoder *decoder, const void *src, lv_img_header_t *header);
static lv_res_t decoder_open(lv_img_decoder_t *dec, lv_img_decoder_dsc_t *dsc);
static void decoder_close(lv_img_decoder_t *dec, lv_img_decoder_dsc_t *dsc);
static void convert_color_depth(uint8_t *img, uint32_t px_cnt);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Register the PNG decoder functions in LittlevGL
 */
void lv_png_init(const unsigned char *rom_bin, unsigned int size)
{
	_lv_ll_init(&LV_GC_ROOT(_lv_img_defoder_ll), sizeof(lv_img_decoder_t));

	lv_img_decoder_t *dec = lv_img_decoder_create();
	lv_img_decoder_set_info_cb(dec, decoder_info);
	lv_img_decoder_set_open_cb(dec, decoder_open);
	lv_img_decoder_set_close_cb(dec, decoder_close);

	png_bin_base = rom_bin;
	png_bin_size = size;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Get info about a PNG image
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_info(struct _lv_img_decoder *decoder, const void *src, lv_img_header_t *header)
{
	(void)decoder; /*Unused*/
	rom_bin_info *png_info = (rom_bin_info *)src;

	if (png_info->type == 0)
	{
		if (png_info->offset > (png_bin_size - 16))
		{
			return LV_RES_INV; /*If didn't succeeded earlier then it's an error*/
		}

		uint32_t *src_addres = (uint32_t *)(png_bin_base + png_info->offset + 16);

		header->always_zero = 0;
		header->cf = LV_IMG_CF_RAW_ALPHA;

		header->w = (lv_coord_t)((src_addres[0] & 0xff000000) >> 24) + ((src_addres[0] & 0x00ff0000) >> 8);
		header->h = (lv_coord_t)((src_addres[1] & 0xff000000) >> 24) + ((src_addres[1] & 0x00ff0000) >> 8);

		if ((header->w > 1024) || (header->h > 600))
		{
			return LV_RES_INV;
		}
	}
	else if (png_info->type == 1)
	{
		header->always_zero = 0;
		header->cf = LV_IMG_CF_RAW;
		header->w = png_info->size & 0xFFF;
		header->h = (png_info->size >> 12) & 0xFFF;
		// LV_LOG_WARN("decode raw data \n");
	}
	//	printf("decode start %dx%d\n",header->w,header->h);
	return LV_RES_OK;
#if 0	
     lv_img_src_t src_type = lv_img_src_get_type(src);          /*Get the source type*/

     /*If it's a PNG file...*/
     if(src_type == LV_IMG_SRC_FILE)
	 {
         const char * fn = src;
         if(!strcmp(&fn[strlen(fn) - 3], "png"))
		 {              /*Check the extension*/

             /* Read the width and height from the file. They have a constant location:
              * [16..23]: width
              * [24..27]: height
              */
             uint32_t size[2];
#if LV_PNG_USE_LV_FILESYSTEM
             lv_fs_file_t f;
             lv_fs_res_t res = lv_fs_open(&f, fn, LV_FS_MODE_RD);
             if(res != LV_FS_RES_OK) return LV_RES_INV;
             lv_fs_seek(&f, 16, LV_FS_SEEK_SET);
             uint32_t rn;
             lv_fs_read(&f, &size, 8, &rn);
             if(rn != 8) return LV_RES_INV;
             lv_fs_close(&f);
#else
             FILE* file;
             file = fopen(fn, "rb" );
             if(!file) return LV_RES_INV;
             fseek(file, 16, SEEK_SET);
             size_t rn = fread(size, 1 , 8, file);
             fclose(file);
             if(rn != 8) return LV_RES_INV;
#endif
             /*Save the data in the header*/
             header->always_zero = 0;
             header->cf = LV_IMG_CF_RAW_ALPHA;
             /*The width and height are stored in Big endian format so convert them to little endian*/
             header->w = (lv_coord_t) ((size[0] & 0xff000000) >> 24) +  ((size[0] & 0x00ff0000) >> 8);
             header->h = (lv_coord_t) ((size[1] & 0xff000000) >> 24) +  ((size[1] & 0x00ff0000) >> 8);

             return LV_RES_OK;
         }
     }
     /*If it's a PNG file in a  C array...*/
     else if(src_type == LV_IMG_SRC_VARIABLE)
	 {
         const lv_img_dsc_t * img_dsc = src;
         header->always_zero = 0;
         header->cf = img_dsc->header.cf;       /*Save the color format*/
         header->w = img_dsc->header.w;         /*Save the color width*/
         header->h = img_dsc->header.h;         /*Save the color height*/
         return LV_RES_OK;
     }
     return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
#endif
}

/**
 * Open a PNG image and return the decided image
 * @param src can be file name or pointer to a C array
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or  `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static int png_decode_count = 0;
static lv_res_t decoder_open(lv_img_decoder_t *decoder, lv_img_decoder_dsc_t *dsc)
{
	(void)decoder;	/*Unused*/
	uint32_t error; /*For the return values of PNG decoder functions*/

	if (dsc->img_data)
	{
		return LV_RES_OK;
	}
	rom_bin_info *png_info = (rom_bin_info *)dsc->src;
	if (png_info->size < sizeof(rom_bin_info))
	{
		return LV_RES_INV; /*If didn't succeeded earlier then it's an error*/
	}

	if (png_info->type == 0)
	{
		const unsigned char *png_data = (png_bin_base + png_info->offset);
		size_t png_data_size = png_info->size;

		uint32_t png_width;
		uint32_t png_height;
		uint8_t *img_data = NULL;
		error = lodepng_decode32(&img_data, &png_width, &png_height, png_data, png_data_size);
		if (error)
		{
			printf("error %u: %s\n", error, lodepng_error_text(error));
			return LV_RES_INV;
		}
		//	printf("%s:%d decode ing. %dx%d \n",__func__,__LINE__,png_width,png_height);
		/*Convert the image to the system's color depth*/
		convert_color_depth(img_data, png_width * png_height);
		dsc->img_data = img_data;
	}
	else if (png_info->type == 1)
	{
		dsc->img_data = ((unsigned char *)png_info->offset);
	}
	png_decode_count++;
	// printf("decode png :%d type:%d\n", png_decode_count, png_info->type);
	return LV_RES_OK; /*Return with its pointer*/
#if 0

    uint8_t * img_data = NULL;

    /*If it's a PNG file...*/
    if(dsc->src_type == LV_IMG_SRC_FILE) {
        const char * fn = dsc->src;

        if(!strcmp(&fn[strlen(fn) - 3], "png")) {              /*Check the extension*/

            /*Load the PNG file into buffer. It's still compressed (not decoded)*/
            unsigned char * png_data;      /*Pointer to the loaded data. Same as the original file just loaded into the RAM*/
            size_t png_data_size;          /*Size of `png_data` in bytes*/

            error = lodepng_load_file(&png_data, &png_data_size, fn);   /*Load the file*/
            if(error) {
                printf("error %u: %s\n", error, lodepng_error_text(error));
                return LV_RES_INV;
            }

            /*Decode the PNG image*/
            uint32_t png_width;             /*Will be the width of the decoded image*/
            uint32_t png_height;            /*Will be the width of the decoded image*/

            /*Decode the loaded image in ARGB8888 */
            error = lodepng_decode32(&img_data, &png_width, &png_height, png_data, png_data_size);
            free(png_data); /*Free the loaded file*/
            if(error) {
                printf("error %u: %s\n", error, lodepng_error_text(error));
                return LV_RES_INV;
            }

            /*Convert the image to the system's color depth*/
            convert_color_depth(img_data,  png_width * png_height);
            dsc->img_data = img_data;
            return LV_RES_OK;     /*The image is fully decoded. Return with its pointer*/
        }
    }
    /*If it's a PNG file in a  C array...*/
    else if(dsc->src_type == LV_IMG_SRC_VARIABLE) {
        const lv_img_dsc_t * img_dsc = dsc->src;
        uint32_t png_width;             /*No used, just required by he decoder*/
        uint32_t png_height;            /*No used, just required by he decoder*/

        /*Decode the image in ARGB8888 */
        error = lodepng_decode32(&img_data, &png_width, &png_height, img_dsc->data, img_dsc->data_size);

        if(error) {
            return LV_RES_INV;
        }

        /*Convert the image to the system's color depth*/
        convert_color_depth(img_data,  png_width * png_height);

        dsc->img_data = img_data;
        return LV_RES_OK;     /*Return with its pointer*/
    }

    return LV_RES_INV;    /*If not returned earlier then it failed*/
#endif
}

/**
 * Free the allocated resources
 */
static void decoder_close(lv_img_decoder_t *decoder, lv_img_decoder_dsc_t *dsc)
{
	rom_bin_info *png_info = (rom_bin_info *)dsc->src;

	png_decode_count--;
	// printf("close decode png :%d  type:%d\n", png_decode_count, png_info->type);
	(void)decoder; /*Unused*/
	if (dsc->img_data)
	{
		if (png_info->type == 0)
		{
			free((uint8_t *)dsc->img_data);
		}
		dsc->img_data = NULL;
	}
}

/**
 * If the display is not in 32 bit format (ARGB888) then covert the image to the current color depth
 * @param img the ARGB888 image
 * @param px_cnt number of pixels in `img`
 */
static void convert_color_depth(uint8_t *img, uint32_t px_cnt)
{
#if LV_COLOR_DEPTH == 32
	lv_color32_t *img_argb = (lv_color32_t *)img;
	lv_color_t c;
	lv_color_t *img_c = (lv_color_t *)img;
	uint32_t i;
	for (i = 0; i < px_cnt; i++)
	{
		c = lv_color_make(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
		img_c[i].ch.red = c.ch.blue;
		img_c[i].ch.blue = c.ch.red;
	}
#elif LV_COLOR_DEPTH == 16
	lv_color32_t *img_argb = (lv_color32_t *)img;
	lv_color_t c;
	uint32_t i;
	for (i = 0; i < px_cnt; i++)
	{
		c = lv_color_make(img_argb[i].ch.blue, img_argb[i].ch.green, img_argb[i].ch.red);
		img[i * 3 + 2] = img_argb[i].ch.alpha;
		img[i * 3 + 1] = c.full >> 8;
		img[i * 3 + 0] = c.full & 0xFF;
	}
#elif LV_COLOR_DEPTH == 8
	lv_color32_t *img_argb = (lv_color32_t *)img;
	lv_color_t c;
	uint32_t i;
	for (i = 0; i < px_cnt; i++)
	{
		c = lv_color_make(img_argb[i].red, img_argb[i].green, img_argb[i].blue);
		img[i * 2 + 1] = img_argb[i].alpha;
		img[i * 2 + 0] = c.full
	}
#endif
}
