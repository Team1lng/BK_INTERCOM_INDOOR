#include "layout_define.h"
#include "leo_api.h"

extern void carr_record_way_set(media_type way);


typedef enum event_module_list
{
	CALL_MODULE,
	MESSAGE_MODULE,
	MOTION_MODULE,
#ifdef ALARM_MODULE_ENABLE
	ALARM_MODULE,
#endif
	TOTAL_MODULE
}event_module_list;

static void  CALL_MODULE_UP(lv_obj_t *obj)
{
	carr_record_way_set(FILE_TYPE_SD_CALL);
	goto_layout(pLAYOUT(file_list));
}

static lv_obj_t * CALL_MODULE_CREATE(user_obj* obj)
{
    static rom_bin_info info = rom_bin_info_get(ROM_RES_EVENT_CALL_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_EVENT_CALL_FOCUS_PNG);
	return home_btn_create_1(obj->area,text_str(STR_CALL_RECORD),&obj->btn_data,&info,&info1);
}


static void MESSAGE_MODULE_UP(lv_obj_t *obj)
{
	carr_record_way_set(FILE_TYPE_SD_MSG);
	goto_layout(pLAYOUT(file_list));
}

static lv_obj_t * MESSAGE_MODULE_CREATE(user_obj* obj)
{
    static rom_bin_info info = rom_bin_info_get(ROM_RES_EVENT_MESSAGE_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_EVENT_MESSAGE_FOCUS_PNG);
	return home_btn_create_1(obj->area,text_str(STR_MESSAGE_RECORD),&obj->btn_data,&info,&info1);
}


static void MOTION_MODULE_UP(lv_obj_t *obj)
{
	carr_record_way_set(FILE_TYPE_SD_MOTION);
	goto_layout(pLAYOUT(file_list));
}

static lv_obj_t * MOTION_MODULE_CREATE(user_obj* obj)
{
    static rom_bin_info info = rom_bin_info_get(ROM_RES_EVENT_MOTION_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_EVENT_MOTION_FOCUS_PNG);
	return home_btn_create_1(obj->area,text_str(STR_MOTION_RECORD),&obj->btn_data,&info,&info1);
}

#ifdef ALARM_MODULE_ENABLE
static void ALARM_MODULE_UP(lv_obj_t *obj)
{
	carr_record_way_set(FILE_TYPE_SD_ALARM);
	goto_layout(pLAYOUT(file_list));
}

static lv_obj_t * ALARM_MODULE_CREATE(user_obj* obj)
{	
    static rom_bin_info info = rom_bin_info_get(ROM_RES_EVENT_ALARM_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_EVENT_ALARM_FOCUS_PNG);
	return home_btn_create_1(obj->area,text_str(STR_ALARM_RECORD),&obj->btn_data,&info,&info1);
}
#endif

static void event_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

USER_OBJ_INIT(MAIN_OBJ,TOTAL_MODULE)
{
	[CALL_MODULE] = {.area = {336,100,130,130}, .parent = NULL, USER_OBJ_CONSTRUCTOR_OPS(CALL_MODULE,UP)},
	[MESSAGE_MODULE] = {.area = {574,100,130,130}, .parent = NULL, USER_OBJ_CONSTRUCTOR_OPS(MESSAGE_MODULE,UP)},
	[MOTION_MODULE] = {.area = {336,320,130,130}, .parent = NULL, USER_OBJ_CONSTRUCTOR_OPS(MOTION_MODULE,UP)},
#ifdef ALARM_MODULE_ENABLE
	[ALARM_MODULE] = {.area = {574,320,130,130}, .parent = NULL, USER_OBJ_CONSTRUCTOR_OPS(ALARM_MODULE,UP)},
#endif
};

static void LAYOUT_ENETER_FUNC(event)
{
	home_bg_display();

	USER_OBJ_GROUP_CREATE(MAIN_OBJ);
	home_back_btn_create(event_back_btn_up,NULL);
	
}


static void LAYOUT_QUIT_FUNC(event)
{
	USER_OBJ_GROUP_DESTROY(MAIN_OBJ);
}


CREATE_LAYOUT(event);


