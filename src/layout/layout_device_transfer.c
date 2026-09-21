#include "layout_define.h"
#include "leo_api.h"

static lv_obj_t*  Device_transfer_obj_create(btn_data *btn,void *img_src1,void *img_src2);
static void Device_transfer_obj_up(lv_obj_t *obj);
static int device_create_num = 0;
static int device_online_index = 0;
static int device_x_offset = 0;
extern int call_family_id;
extern network_device call_num;
extern interphone_status_enum call_status;

#define TAG_LIST(tag) \
tag(EXTENSION1)\
tag(EXTENSION2)\
tag(EXTENSION3)\
tag(EXTENSION4)\
tag(EXTENSION5)\
tag(EXTENSION6)\
tag(EXTENSION_ALL)\

#define DEFINE_TAG(_tag) _tag,
typedef enum  {
    None = 0,
    TAG_LIST(DEFINE_TAG)
    EmMAX
}Obj_Module;

#define Device_Operater(Device) \
static void Device##_UP(lv_obj_t *obj) {\
    Device_transfer_obj_up(obj);\
}\
static void  Device##_CREATE()  {\
    Debug("%d\n",Device);\
    if(family_dev_online_state_get(call_family_id,Device) == false && Device != EXTENSION_ALL)\
        return;\
    if(user_data_get()->other.family_id == call_family_id && user_data_get()->other.network_device == Device)\
        return;\
	static btn_data btn_data = btn_data_create(NULL, Device##_UP, NULL);\
    lv_obj_t* obj = Device_transfer_obj_create(&btn_data,&Device##_UNFOCUS,&Device##_FOCUS);\
    lv_obj_set_id(obj,Device);\
}

#define Device_Res(tag) \
static rom_bin_info tag##_UNFOCUS = rom_bin_info_get(ROM_RES_TRANSFER_##tag##_UNFOCUS_PNG);\
static rom_bin_info tag##_FOCUS = rom_bin_info_get(ROM_RES_TRANSFER_##tag##_FOCUS_PNG);\

TAG_LIST(Device_Res)
TAG_LIST(Device_Operater)

#define DEVICE_OBJ_INTERVAL 40
#define DEVICE_OBJ_SIZE 77
#define DEVICE_OBJ_X_OFFSET ((1024 - DEVICE_OBJ_SIZE*(device_create_num) - DEVICE_OBJ_INTERVAL*(device_create_num-1))/2)
#define DEVICE_OBJ_Y_OFFSET 261

static lv_obj_t*  Device_transfer_obj_create(btn_data *btn,void *img_src1,void *img_src2)
{
    Controls_location area;
    area.x = device_x_offset + device_online_index*(DEVICE_OBJ_SIZE+DEVICE_OBJ_INTERVAL);
    area.y = DEVICE_OBJ_Y_OFFSET;
    area.width = area.high = DEVICE_OBJ_SIZE;
    device_online_index ++;
    Debug("%d,%d,%d,%d\n",area.x,device_x_offset,device_create_num,device_online_index);
    return home_btn_create_2(area,btn,img_src1,img_src2);
}

static void Device_transfer_obj_up(lv_obj_t *obj)
{
    if(obj->obj_id != EXTENSION_ALL)
    {
        call_num = obj->obj_id;
    }
    else
    {
        call_num = DEVICE_ALL;
    }
	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void device_obj_create(void)
{
    Debug("%d,%d\n",user_data_get()->other.family_id,call_family_id);
    Debug("%d\n",user_data_get()->other.network_device);
    device_create_num = device_online_index = 0;
    for(network_device device = DEVICE_INDOOR_ID1;device < DEVICE_OUTDOOR_1;device ++)
    {
        if(user_data_get()->other.family_id == call_family_id && user_data_get()->other.network_device == device)
        {
            continue;
        }

        if(family_dev_online_state_get(call_family_id,device))
        {
            device_create_num++;
        }
    }
    /* 还存在一个ALL键 ，因此+1*/
    device_create_num += 1;
    device_x_offset = DEVICE_OBJ_X_OFFSET;
    EXTENSION1_CREATE();
    EXTENSION2_CREATE();
    EXTENSION3_CREATE();
    EXTENSION4_CREATE();
    EXTENSION5_CREATE();
    EXTENSION6_CREATE();
    EXTENSION_ALL_CREATE();
}

static void back_btn_up(lv_obj_t *obj)
{
    #ifdef FAMILY_TRANSFER_MODULE
    goto_layout(pLAYOUT(family_transfer));
    #else
    goto_layout(pLAYOUT(home));
    #endif
}

static void LAYOUT_ENETER_FUNC(device_transfer)
{
	home_bg_display();
	home_back_btn_create(back_btn_up,NULL);
    device_obj_create();
}

static void LAYOUT_QUIT_FUNC(device_transfer)
{
  
}

CREATE_LAYOUT(device_transfer);