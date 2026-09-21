/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-08-05 11:53:58
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-10-18 10:23:17
 * @FilePath: /two-wire-indoor/src/layout/user_obj.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _USER_OBJ_H_
#define _USER_OBJ_H_

#include "../lvgl/lvgl.h"

#define USER_OBJ_DECLARATION(OBJ_GROUP,TOTAL) static user_obj (OBJ_GROUP)[TOTAL];

#define USER_OBJ_INIT(OBJ_GROUP,TOTAL) static user_obj (OBJ_GROUP)[TOTAL] =
																							  
#define USER_OBJ_GET(OBJ_GROUP,obj) (OBJ_GROUP)[obj]
#define USER_OBJ_CONSTRUCTOR(obj)	.id = obj, .constructor = obj##_CREATE, 
#define USER_OBJ_CONSTRUCTOR_OPS(obj, ops)	.id = obj, .constructor = obj##_CREATE, \
                                                                                      .btn_data.OPS_##ops = obj##_##ops,\


typedef struct controls_coordinate
{
	int x;
	int y;
	int width;
	int high;
}Controls_location;
typedef Controls_location  obj_area;

typedef struct
{
	bool obj_tone;
	void *user_data;
	void (*OPS_DOWN)(lv_obj_t *obj);
	void (*OPS_UP)(lv_obj_t *obj);
	void (*OPS_ANYTHING)(lv_obj_t *obj, lv_event_t event);
} btn_data;

#define USER_OBJ_GROUP_CREATE(OBJ_GROUP)	\
    do { \
        for (int i = 0; i < sizeof((OBJ_GROUP)) / sizeof((OBJ_GROUP)[0]); i++) { \
            user_obj *obj = &(OBJ_GROUP)[i]; \
            obj->obj = obj->constructor(obj); \
        } \
    } while(0)

#define USER_OBJ_GROUP_DESTROY(OBJ_GROUP)	\
    do { \
        for (int i = 0; i < sizeof((OBJ_GROUP)) / sizeof((OBJ_GROUP)[0]); i++) { \
            user_obj *obj = &(OBJ_GROUP)[i]; \
            obj->obj = NULL; \
        } \
    } while(0)

#define USER_OBJ_CREATE(OBJ)	\
    do { \
            OBJ->obj = OBJ->constructor(obj); \
    } while(0)

typedef struct UserObj
{
	unsigned int id;

	obj_area area;
	btn_data btn_data;

	lv_obj_t *obj;
	struct UserObj *parent;
	
	lv_obj_t * (*constructor)(struct UserObj *obj);
}user_obj;

#endif