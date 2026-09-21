#include "layout_define.h"
#include "leo_api.h"



// #define MOVE_FILE_MODULE_ENABLE 

typedef enum media_module_list
{
#ifdef MOVE_FILE_MODULE_ENABLE
	MOVIE_MODULE,
#endif

	MUSIC_MODULE,
	PHOTO_MODULE,


#ifdef MOVE_FILE_MODULE_ENABLE
	FILES_MODULE,
#endif
	TOTAL_MODULE
}media_module_list;

#ifdef MOVE_FILE_MODULE_ENABLE
#define MEDIA_MODULE_COORDINATE_INIT  {\
			{336,100,130,130},\
			{574,100,130,130},\
			{336,320,130,130},\
			{574,320,130,130},\
	};

#else
	#define MEDIA_MODULE_COORDINATE_INIT  {\
				{336,210,130,130},\
				{574,210,130,130},\
		};
#endif


#ifdef MOVE_FILE_MODULE_ENABLE
static void media_movie_btn_up(lv_obj_t *obj)
{
	// goto_layout(pLAYOUT(music_list));
}

static void media_movie_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, media_movie_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_MOVIE_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_MOVIE_FOCUS_PNG);
	home_btn_create_1(coordinate,text_str(STR_MEDIA),&btn_data,&info,&info1);
}
#endif

static void media_music_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(music_list));
}

static void media_music_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, media_music_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_MUSIC_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_MUSIC_FOCUS_PNG);
	home_btn_create_1(coordinate,text_str(STR_MUSIC),&btn_data,&info,&info1);
}


static void media_photo_btn_up(lv_obj_t *obj)
{	
	goto_layout(pLAYOUT(photo_list));
}

static void media_photo_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, media_photo_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_PHOTO_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_PHOTO_FOCUS_PNG);
	home_btn_create_1(coordinate,text_str(STR_PHOTO),&btn_data,&info,&info1);
}


#ifdef MOVE_FILE_MODULE_ENABLE

static void media_files_btn_up(lv_obj_t *obj)
{
	
}

static void media_files_btn_create(Controls_location coordinate)
{
	static btn_data btn_data = btn_data_create(NULL, media_files_btn_up, NULL);
    static rom_bin_info info = rom_bin_info_get(ROM_RES_MEDIA_FILE_UNFOCUS_PNG);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_MEDIA_FILE_FOCUS_PNG);
	home_btn_create_1(coordinate,text_str(STR_FILES),&btn_data,&info,&info1);
}
#endif

static void media_back_btn_up(lv_obj_t *obj)
{
    goto_layout(pLAYOUT(home));
}


static void LAYOUT_ENETER_FUNC(media)
{
	Controls_location module_coordinate[] =  MEDIA_MODULE_COORDINATE_INIT;
	home_bg_display();

#ifdef MOVE_FILE_MODULE_ENABLE
	media_movie_btn_create(module_coordinate[MOVIE_MODULE]);
	media_files_btn_create(module_coordinate[FILES_MODULE]);
#endif

	media_music_btn_create(module_coordinate[MUSIC_MODULE]);

	media_photo_btn_create(module_coordinate[PHOTO_MODULE]);


	home_back_btn_create(media_back_btn_up,NULL);

	
}


static void LAYOUT_QUIT_FUNC(media)
{
    
}


CREATE_LAYOUT(media);


