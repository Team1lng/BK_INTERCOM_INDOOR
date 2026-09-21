#include "layout_define.h"
#include "leo_api.h"

typedef enum transfer_module_list
{
	CALL_DEV1_MODULE,
	CALL_DEV2_MODULE,
	CALL_DEV3_MODULE,
	CALL_DEV4_MODULE,
	CALL_DEV5_MODULE,
	CALL_ALL_MODULE,
	TOTAL_MODULE
}transfer_module_list;

#define TRANSFER_MODULE_COORDINATE_INIT  {\
			{181,261,77,77},\
			{298,261,77,77},\
			{415,261,77,77},\
			{532,261,77,77},\
			{649,261,77,77},\
			{766,261,77,77},\
	};

extern network_device call_num;
extern int call_family_id;
extern interphone_status_enum call_status;

static rom_bin_info* call_dev_info_get(int cont,int is_focus){
	if(cont == 1){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION1_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION1_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 2){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION2_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION2_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 3){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION3_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION3_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 4){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION4_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION4_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 5){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION5_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION5_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 6){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION6_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION6_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}else if(cont == 7){
		static rom_bin_info info = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION_ALL_FOCUS_PNG);
		static rom_bin_info info1 = rom_bin_info_get(ROM_RES_TRANSFER_EXTENSION_ALL_UNFOCUS_PNG);
		if(is_focus){
			return &info;
		}else{
			return &info1;
		}
	}
	return NULL;
}


static void call_dev_1_btn_up(lv_obj_t *obj)
{
		
	if(user_data_get()->other.network_device == 1){
		call_num = DEVICE_INDOOR_ID2;
	}else{
		call_num = DEVICE_INDOOR_ID1;
	}

	// if(device_online_state_get(call_num) == false)
	// 	return;

	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_1_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_1_btn_up, NULL);
	if(user_data_get()->other.network_device == 1){
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(2,0),call_dev_info_get(2,1));
	}else{
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(1,0),call_dev_info_get(1,1));
	}
}

static void call_dev_2_btn_up(lv_obj_t *obj)
{
		
	if(user_data_get()->other.network_device <= 2){
		call_num = DEVICE_INDOOR_ID3;
	}else{
		call_num = DEVICE_INDOOR_ID2;
	}

	// if(device_online_state_get(call_num) == false)
	// 	return;

	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_2_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_2_btn_up, NULL);
	if(user_data_get()->other.network_device <= 2){
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(3,0),call_dev_info_get(3,1));
	}else{
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(2,0),call_dev_info_get(2,1));
	}
}

static void call_dev_3_btn_up(lv_obj_t *obj)
{
	if(user_data_get()->other.network_device <= 3){
		call_num = DEVICE_INDOOR_ID4;
	}else{
		call_num = DEVICE_INDOOR_ID3;
	}

	// if(device_online_state_get(call_num) == false)
	// 	return;

	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_3_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_3_btn_up, NULL);
	if(user_data_get()->other.network_device <= 3){
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(4,0),call_dev_info_get(4,1));
	}else{
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(3,0),call_dev_info_get(3,1));
	}
}


static void call_dev_4_btn_up(lv_obj_t *obj)
{
	if(user_data_get()->other.network_device <= 4){
		call_num = DEVICE_INDOOR_ID5;
	}else{
		call_num = DEVICE_INDOOR_ID4;
	}

	// if(device_online_state_get(call_num) == false)
	// 	return;

	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_4_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_4_btn_up, NULL);
	if(user_data_get()->other.network_device <= 4){
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(5,0),call_dev_info_get(5,1));
	}else{
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(4,0),call_dev_info_get(4,1));
	}
}

static void call_dev_5_btn_up(lv_obj_t *obj)
{
	if(user_data_get()->other.network_device <= 5){
		call_num = DEVICE_INDOOR_ID6;
	}else{
		call_num = DEVICE_INDOOR_ID5;
	}

	// if(device_online_state_get(call_num) == false)
	// 	return;

	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_5_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_5_btn_up, NULL);
	if(user_data_get()->other.network_device <= 5){
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(6,0),call_dev_info_get(6,1));
	}else{
		home_btn_create_2(coordinate,&btn_data,call_dev_info_get(5,0),call_dev_info_get(5,1));
	}
}

static void call_dev_all_btn_up(lv_obj_t *obj)
{
	call_num = DEVICE_ALL;
	call_status = INTERPHONE_STATUS_IDLE;
    goto_layout(pLAYOUT(call));
}

static void call_dev_all_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, call_dev_all_btn_up, NULL);
	home_btn_create_2(coordinate,&btn_data,call_dev_info_get(7,0),call_dev_info_get(7,1));

}

static void back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}

static void LAYOUT_ENETER_FUNC(transfer)
{
	Controls_location module_coordinate[] =  TRANSFER_MODULE_COORDINATE_INIT;
	home_bg_display();
	home_back_btn_create(back_btn_up,NULL);
	call_dev_1_btn_create(module_coordinate[CALL_DEV1_MODULE]);
	call_dev_2_btn_create(module_coordinate[CALL_DEV2_MODULE]);
	call_dev_3_btn_create(module_coordinate[CALL_DEV3_MODULE]);
	call_dev_4_btn_create(module_coordinate[CALL_DEV4_MODULE]);
	call_dev_5_btn_create(module_coordinate[CALL_DEV5_MODULE]);
	call_dev_all_btn_create(module_coordinate[CALL_ALL_MODULE]);
}
	
static void LAYOUT_QUIT_FUNC(transfer)
{
  
}


CREATE_LAYOUT(transfer);





























