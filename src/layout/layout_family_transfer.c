#include "layout_define.h"
#include "leo_api.h"

#ifdef FAMILY_TRANSFER_MODULE 
static lv_obj_t*  family_transfer_obj_create(btn_data *btn,void *img_src1,void *img_src2);
static void family_transfer_obj_up(lv_obj_t *obj);
static bool family_online_status(network_family family);
static int family_online_num = 0;
static int family_online_index = 0;
static int family_x_offset = 0;

#define TAG_LIST(tag) \
tag(FAMILY1)\
tag(FAMILY2)\
tag(FAMILY3)\
tag(FAMILY4)\

#define DEFINE_TAG(_tag) _tag,
typedef enum  {
    None = 0,
    TAG_LIST(DEFINE_TAG)
    EmMAX
}Obj_Module;

#define Family_Operater(Family) \
static void Family##_UP(lv_obj_t *obj) {\
    family_transfer_obj_up(obj);\
}\
static void  Family##_CREATE()  {\
    Debug("%d\n",Family);\
    if(family_online_status(Family) == false)\
        return;\
	static btn_data btn_data = btn_data_create(NULL, Family##_UP, NULL);\
    lv_obj_t* obj = family_transfer_obj_create(&btn_data,&Family##_UNFOCUS,&Family##_FOCUS);\
    lv_obj_set_id(obj,Family);\
}

#define Family_Res(tag) \
static rom_bin_info tag##_UNFOCUS = rom_bin_info_get(ROM_RES_TRANSFER_##tag##_PNG);\
static rom_bin_info tag##_FOCUS = rom_bin_info_get(ROM_RES_TRANSFER_##tag##_FOCUS_PNG);\

TAG_LIST(Family_Res)
TAG_LIST(Family_Operater)

#define FAMILY_OBJ_INTERVAL 50
#define FAMILY_OBJ_SIZE 102
#define FAMILY_OBJ_X_OFFSET ((1024 - FAMILY_OBJ_SIZE*(family_online_num) - FAMILY_OBJ_INTERVAL*(family_online_num-1))/2)
#define FAMILY_OBJ_Y_OFFSET 249

static lv_obj_t*  family_transfer_obj_create(btn_data *btn,void *img_src1,void *img_src2)
{
    Controls_location area;
    area.x = family_x_offset + family_online_index*(FAMILY_OBJ_SIZE+FAMILY_OBJ_INTERVAL);
    area.y = FAMILY_OBJ_Y_OFFSET;
    area.width = area.high = FAMILY_OBJ_SIZE;
    family_online_index ++;
    Debug("%d,%d,%d\n",area.x,family_x_offset,family_online_num);
    return home_btn_create_2(area,btn,img_src1,img_src2);
}

static void family_transfer_obj_up(lv_obj_t *obj)
{
    extern int call_family_id;
    call_family_id = obj->obj_id;
	goto_layout(pLAYOUT(device_transfer));
}

static void back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

static bool family_online_status(network_family family)
{
    for(network_device dev = DEVICE_INDOOR_ID1;dev < DEVICE_OUTDOOR_1;dev ++)
    {
        if(user_data_get()->other.family_id == family && user_data_get()->other.network_device == dev)
            continue;
            
        if(family_dev_online_state_get(family,dev))
            return true;
    }
    return false;
} 

static void family_obj_create(void)
{
    family_online_num = family_online_index = 0;
    for(network_family family = FAMILY_ID1;family < FAMILY_TOTAL;family ++)
    {
        if(family_online_status(family))
        {
            family_online_num++;
        }
    }
    family_x_offset = FAMILY_OBJ_X_OFFSET;
    FAMILY1_CREATE();
    FAMILY2_CREATE();
    FAMILY3_CREATE();
    FAMILY4_CREATE();
}

static void LAYOUT_ENETER_FUNC(family_transfer)
{
	home_bg_display();
	home_back_btn_create(back_btn_up,NULL);
    family_obj_create();
}

static void LAYOUT_QUIT_FUNC(family_transfer)
{
  
}

CREATE_LAYOUT(family_transfer);
#endif

