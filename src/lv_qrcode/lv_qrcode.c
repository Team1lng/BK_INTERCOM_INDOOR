/**
 * @file lv_qrcode.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_qrcode.h"
#include "qrcodegen.h"
#include "../lvgl/src/lv_widgets/lv_canvas.h"
/*********************
 *      DEFINES
 *********************/
#define QR_SIZE     140

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

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
 * Create an empty QR code (an `lv_canvas`) object.
 * @param parent point to an object where to create the QR code
 * @param size width and height of the QR code
 * @param dark_color dark color of the QR code
 * @param light_color light color of the QR code
 * @return pointer to the created QR code object
 */
lv_obj_t *lv_qrcode_create(lv_obj_t *parent, void *data, lv_coord_t size, lv_color_t dark_color, lv_color_t light_color)
{
	lv_obj_t *canvas = lv_canvas_create(parent, NULL);
	if (canvas == NULL)
		return NULL;
	lv_img_set_src(canvas, data);

	lv_img_dsc_t *imgdsc = lv_canvas_get_img(canvas);
	imgdsc->header.cf = LV_IMG_CF_RAW;
	imgdsc->header.w = size;
	imgdsc->header.h = size;
	rom_bin_info *img = (rom_bin_info *)(data);
	imgdsc->data = ((unsigned char *)img->offset);

	lv_canvas_set_palette(canvas, 0, dark_color);
	lv_canvas_set_palette(canvas, 1, light_color);
	return canvas;
}

/**
 * Set the data of a QR code object
 * @param qrcode pointer to aQ code object
 * @param data data to display
 * @param data_len length of data in bytes
 * @return LV_RES_OK: if no error; LV_RES_INV: on error
 */
lv_res_t lv_qrcode_update(lv_obj_t * qrcode, const void * data, uint32_t data_len)
{
	lv_color_t c;
	c.full = 1;
	lv_canvas_fill_bg(qrcode, c, LV_OPA_COVER);
	if (data_len > qrcodegen_BUFFER_LEN_MAX)
		return LV_RES_INV;
	uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];
	uint8_t data_tmp[qrcodegen_BUFFER_LEN_MAX];
	memcpy(data_tmp, data, data_len);
	bool ok = qrcodegen_encodeBinary(data_tmp, data_len,
					 qr0, qrcodegen_Ecc_MEDIUM,
					 qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
					 qrcodegen_Mask_AUTO, true);

	if (!ok)
		return LV_RES_INV;

	lv_img_ext_t *imgdsc = lv_obj_get_ext_attr(qrcode);
	lv_coord_t obj_w = imgdsc->w;
	int qr_size = qrcodegen_getSize(qr0);
	int scale = obj_w / qr_size;
	int scaled = qr_size * scale;
	int margin = (obj_w - scaled) / 2;

	rom_bin_info *img = (rom_bin_info *)(imgdsc->src);
	unsigned char *buf_u8 = ((unsigned char *)img->offset);

	int raw_byte = LV_COLOR_SIZE / 8;
	uint32_t row_byte_cnt = obj_w * raw_byte;

	lv_color32_t d_c32;
	lv_color32_t l_c32;
	_lv_memcpy_small(&d_c32, &buf_u8[0 * sizeof(d_c32)], sizeof(d_c32));
	lv_color_t d_color = lv_color_hex(d_c32.full);

	_lv_memcpy_small(&l_c32, &buf_u8[1 * sizeof(l_c32)], sizeof(l_c32));
	lv_color_t l_color = lv_color_hex(l_c32.full);

	//_lv_memcpy_small(buf_u8, &(d_color.full), raw_byte);
	for (int y = 0; y < obj_w; y++)
	{
		for (int x = 0; x < obj_w; x++)
		{
			bool a = qrcodegen_getModule(qr0, (x - margin) / scale, (y - margin) / scale);
			if (a == false)
			{
				_lv_memcpy_small(&buf_u8[y * row_byte_cnt + x * raw_byte], &(l_color.full), raw_byte);
			}
			else
			{
				_lv_memcpy_small(&buf_u8[y * row_byte_cnt + x * raw_byte], &(d_color.full), raw_byte);
			}
		}
	}
	lv_obj_invalidate(qrcode);
	return LV_RES_OK;
}

/**
 * Delete a QR code object
 * @param qrcode pointer to a QR code obejct
 */
void lv_qrcode_delete(lv_obj_t *qrcode)
{
	lv_img_dsc_t *img = lv_canvas_get_img(qrcode);
	lv_img_cache_invalidate_src(img);
	//	lv_mem_free(img->data);
	lv_obj_del(qrcode);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

