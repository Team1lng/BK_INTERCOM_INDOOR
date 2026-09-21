#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define ID3V1_SIZE 128

#if 1
#define wxj_log(format, ...) printf("[DEBUG:%s->%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define wxj_log(format, ...)
#endif

typedef struct
{
	char version;  // 版本号
	char revision; // 副版本号
	// 标志字节定义如:abc00000
	bool flag_a;	// 表示是否使用Unsynchronisation，一般没有
	bool flag_b;	// 表示是否有扩展头部，一般没有
	bool flag_c;	// 表示是否为测试标签，一般没有
	int ID3V2_size; // 标签帧长度
} id3v2_info_t;

// Mp3帧头(FRAMEHEADER)格式如下:AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
typedef struct
{
	unsigned char version_id;		 // BB
	unsigned char layer_num;		 // CC
	unsigned char bitrate_index;	 // EEEE
	unsigned char sample_rate_index; // FF
	unsigned char padding_bit;		 // G
	unsigned char channel_mode;		 // II
} first_frame_info_t;

typedef struct
{
	bool ID3V2_flag;	// id3v2标签帧存在标志位
	bool ID3V1_flag;	// id3v1标签帧存在标志位
	int bitrate;		// 比特率
	int sample_rate;	// 采样率
	int sample_num;		// 采样数
	int frame_size;		// 帧大小
	int frame_num;		// 帧数
	int side_info_size; // 边信息大小
	int total_time;		// 总时长
} mp3_info_t;

const int BitrateTable[5][15] =
	{
		{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448}, // MPEG1 	layer1
		{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},	// MPEG1 	layer2
		{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320},		// MPEG1 	layer3
		{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},	// MPEG2&2.5 	layer1
		{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}			// MPEG2&2.5 	layer2&3
};																				// kbps (0) means :free  0表示自由比特率

const int SampleNumTable[3][3] =
	{
		// MPEG1	MPEG2	MPEG2.5
		{384, 384, 384},	// layer1
		{1152, 1152, 1152}, // layer2
		{1152, 576, 576}	// layer3
};

const int SampleRateTable[3][3] =
	{ // MPEG1	MPEG2	MPEG2.5
		{44100, 48000, 32000},
		{22050, 24000, 16000},
		{11025, 12000, 8000}};

const int SideInfoSizeTable[2][2] =
	{
		// MPEG1	MPEG2/2.5
		{32, 17}, // 立体声	混合立体声	双声
		{17, 9}	  // 单声
};

const char *ChannelTyep[4] = {"立体声", "混合立体声", "双声", "单声"};

/*
获取MP3文件比特率：
	BitRateIndex：BitrateTable数组索引，MP3帧头字节bit[16:27]
	LayerNum:层，MP3帧头字节bit[13:14]
	VersionID：版本，MP3帧头字节bit[11:12]
	return:比特率（位率）
*/
int GetBitRate(unsigned char BitRateIndex, unsigned char LayerNum, unsigned char VersionID)
{
	int i = 0;
	if (VersionID == 3)
	{
		i = 3 - LayerNum;
	}
	else
	{
		if (LayerNum == 3)
			i = 3;
		if (LayerNum == 2 || LayerNum == 1)
			i = 4;
	}
	return BitrateTable[i][BitRateIndex];
}

/*
获取MP3文件每帧采样数（帧大小）：
	LayerNum:层，MP3帧头字节bit[13:14]
	VersionID：版本，MP3帧头字节bit[11:12]
	return:帧大小
*/
int GetSampleNum(unsigned char LayerNum, unsigned char VersionID)
{
	int i = 3 - LayerNum;
	int SampleNum = 0;
	if (VersionID == 3)
		SampleNum = SampleNumTable[i][0];
	if (VersionID == 2)
		SampleNum = SampleNumTable[i][1];
	if (VersionID == 0)
		SampleNum = SampleNumTable[i][2];
	return SampleNum;
}

/*
获取MP3文件每帧采样率：
	SampleRateIndex:SampleRateTable数组索引，MP3帧头字节bit[20:21]
	VersionID：版本，MP3帧头字节bit[11:12]
	return:采样率
*/
int GetSampleRate(unsigned char SampleRateIndex, unsigned char VersionID)
{
	int SampleRate = 0;
	if (VersionID == 3)
		SampleRate = SampleRateTable[0][SampleRateIndex];
	if (VersionID == 2)
		SampleRate = SampleRateTable[1][SampleRateIndex];
	if (VersionID == 0)
		SampleRate = SampleRateTable[2][SampleRateIndex];
	return SampleRate;
}

/*
获取边信息大小：
	ChannelMode:声道模式，MP3帧头字节bit[24:25]
	VersionID：版本，MP3帧头字节bit[11:12]
	return:采样率
*/
int GetSideInfoSize(unsigned char ChannelMode, unsigned char VersionID)
{
	int SideInfoSize = 0;
	if (VersionID == 3)
	{
		if (ChannelMode == 3)
			SideInfoSize = SideInfoSizeTable[1][0];
		else
			SideInfoSize = SideInfoSizeTable[0][0];
	}
	else
	{
		if (ChannelMode == 3)
			SideInfoSize = SideInfoSizeTable[1][1];
		else
			SideInfoSize = SideInfoSizeTable[0][1];
	}
	return SideInfoSize;
}

/*
获取MP3文件信息获取：
	path:文件名
	mp3_info：MP3文件结构
*/
bool music_info_get(char *path, mp3_info_t *mp3_info)
{
	id3v2_info_t id3v2_info;
	first_frame_info_t frame_info;

	int fd;
	int file_size = 0;			   // 整个文件大小
	int total_data_frame_size = 0; // 所有数据帧大小
	char ID3V2_head_buf[10] = {0};
	char ID3V1_buf[128] = {0};

	if (path == NULL)
	{
		wxj_log("File name empty!\n");
		return false;
	}
	int len = strlen(path);

	// if(path[len-4] != '.' || path[len-3] != 'm' || path[len-2] != 'p' || path[len-1] != '3')
	if ((strcmp(&path[len - 4], ".MP3") != 0) && (strcmp(&path[len - 4], ".mp3") != 0)) // 检查文件格式
	{
		wxj_log("File format error!\n");
		return false;
	}

	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		;
		wxj_log("file %s  open error!\n", path);
		return false;
	}

	if (read(fd, ID3V2_head_buf, 10) < 0)
		wxj_log("file read error!\n"); // 获取ID3V2标签头

	if ((ID3V2_head_buf[0] == 'I' || ID3V2_head_buf[0] == 'i') && (ID3V2_head_buf[1] == 'D' || ID3V2_head_buf[1] == 'd') && ID3V2_head_buf[2] == '3') // 检查版本
	{
		mp3_info->ID3V2_flag = true;
		id3v2_info.version = ID3V2_head_buf[3];
		id3v2_info.revision = ID3V2_head_buf[4];
		id3v2_info.flag_a = (ID3V2_head_buf[5] >> 7) & 0x01;
		id3v2_info.flag_b = (ID3V2_head_buf[5] >> 6) & 0x01;
		id3v2_info.flag_c = (ID3V2_head_buf[5] >> 5) & 0x01;
		id3v2_info.ID3V2_size = ((ID3V2_head_buf[6] & 0x7F) << 21 | (ID3V2_head_buf[7] & 0x7F) << 14 | (ID3V2_head_buf[8] & 0x7F) << 7 | (ID3V2_head_buf[9] & 0x7F)) + 10;
	}
	else
	{
		mp3_info->ID3V2_flag = false;
		id3v2_info.ID3V2_size = 0;
		wxj_log("ID3V2 不存在\n");
	}

	file_size = lseek(fd, -128L, SEEK_END) + 128;
	wxj_log("文件大小:%d bytes\n", file_size);

	if (read(fd, ID3V1_buf, ID3V1_SIZE) < 0)
		wxj_log("file read error !\n");
	if ((ID3V1_buf[0] == 'T' || ID3V1_buf[0] == 't') && (ID3V1_buf[1] == 'A' || ID3V1_buf[1] == 'a') && (ID3V1_buf[2] == 'G' || ID3V1_buf[2] == 'g'))
	{
		mp3_info->ID3V1_flag = true;
		wxj_log("ID3V1 长度: [ %d ] bytes\n", ID3V1_SIZE);
		// for(int i=0;i<128;i++) putchar(ID3V1_buf[i]);
		putchar('\n');
	}
	else
	{
		mp3_info->ID3V1_flag = false;
		wxj_log("ID3V1不存在\n");
	}

	int buffer_size = 0, start_pos = 0;
	if (mp3_info->ID3V2_flag == true)
		start_pos = id3v2_info.ID3V2_size;
	else
		start_pos = 0;

	lseek(fd, start_pos, SEEK_SET); // 移文件指针到数据帧的第一帧
	unsigned char *buffer = NULL;

	while ((lseek(fd, 0, SEEK_CUR) + 4) < file_size)
	{
		int cur_pos = lseek(fd, 0, SEEK_CUR);

		if (cur_pos + 1024 <= file_size)
			buffer_size = 1024;
		else
			buffer_size = file_size - cur_pos;

		if (buffer == NULL)
		{
			buffer = malloc(buffer_size);
		}

		if (buffer == NULL) // 检测动态内存分配是否已经成功
		{
			if (fd)
				close(fd);
			wxj_log("ERROR:Memory Exhausted!\n");
			return false;
		}

		memset(buffer, 0, buffer_size);
		read(fd, buffer, buffer_size);
		for (int i = 0; i < (buffer_size - 4); i++)
		{
			if (buffer[i] == 0xFF && buffer[i + 1] > 0xE0)
			{
				// Mp3帧头(FRAMEHEADER)格式如下:AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
				frame_info.version_id = (buffer[i + 1] & 0x18) >> 3; // 'BB' 获取MEPG-ID

				frame_info.layer_num = (buffer[i + 1] & 0x6) >> 1; // 'CC' 获取Layer的索引

				frame_info.bitrate_index = (buffer[i + 2] & 0xF0) >> 4; // 'EEEE' 获取比特率索引

				frame_info.sample_rate_index = (buffer[i + 2] & 0xC) >> 2; // 'FF' 获取采样率索引

				if (frame_info.bitrate_index != 0xF && frame_info.layer_num != 0x0 && frame_info.sample_rate_index < 0x3)
				{
					frame_info.padding_bit = (buffer[i + 2] & 0x2) >> 1;   // 'G' 获取填充位
					frame_info.channel_mode = (buffer[i + 3] & 0xC0) >> 6; // 'II' 获取声道模式索引

					mp3_info->bitrate = GetBitRate(frame_info.bitrate_index, frame_info.layer_num, frame_info.version_id);

					if (mp3_info->bitrate != 0)
					{
						mp3_info->sample_rate = GetSampleRate(frame_info.sample_rate_index, frame_info.version_id);

						mp3_info->sample_num = GetSampleNum(frame_info.layer_num, frame_info.version_id);

						mp3_info->frame_size = ((mp3_info->sample_num / 8 * mp3_info->bitrate * 1000) / mp3_info->sample_rate) + (frame_info.layer_num != 0x03 ? frame_info.padding_bit : (frame_info.padding_bit * 4)); // 计算帧长度

						mp3_info->side_info_size = GetSideInfoSize(frame_info.channel_mode, frame_info.version_id);
						wxj_log("采样率:[%d] 采样数:[%d] 帧大小:[%d] 边信息大小:[%d] 比特率:[%d]\n", mp3_info->sample_rate, mp3_info->sample_num, mp3_info->frame_size, mp3_info->side_info_size, mp3_info->bitrate);
						int XING_head_pos = cur_pos + i + 4 + mp3_info->side_info_size;

						unsigned char str[32];
						memset(str, 0, 32);
						lseek(fd, XING_head_pos, SEEK_SET);
						read(fd, str, 32);

						if (str[0] == 'X' && str[1] == 'i' && str[2] == 'n' && str[3] == 'g')
						{
							wxj_log("XING头存在, 编码方式:VBR（可变比特率）<<<====================\n");

							if (str[7] & 0x01) // 总帧数存储区设置存在，不包含第一帧
							{
								mp3_info->frame_num = ((int)str[8] << 24) | ((int)str[9] << 16) | ((int)str[10] << 8) | (int)str[11];
								wxj_log("XING头附带信息 -->> 总帧数 [ %d ] \n", mp3_info->frame_num);
							}
							else
							{
								mp3_info->frame_num = (int)(total_data_frame_size / mp3_info->frame_size); // 估算帧数
								wxj_log("XING头不含总帧数 -->> 采用公式计算 [ %d ] \n", mp3_info->frame_num);
							}
							mp3_info->total_time = (int)(mp3_info->frame_num * mp3_info->sample_num / (mp3_info->bitrate * 1000)); // 公式计算
						}
						else if (str[0] == 'I' && str[1] == 'n' && str[2] == 'f' && str[3] == 'o')
						{
							wxj_log("Info头存在, 编码方式:CBR（固定比特率）<<<====================\n");

							if (str[7] & 0x02)
							{
								total_data_frame_size = ((int)str[12] << 24) | ((int)str[13] << 16) | ((int)str[14] << 8) | (int)str[15];
								wxj_log("Info头附带信息 -->> 所有数据帧大小 [ %d ] byte\n", total_data_frame_size);
							}
							else
							{
								total_data_frame_size = file_size - (ID3V1_SIZE * (int)(mp3_info->ID3V1_flag & 0x01)) - id3v2_info.ID3V2_size;
								wxj_log("Info头不含数据帧大小信息 -->> 采用公式计算 [ %d ] byte\n", total_data_frame_size);
							}

							if (str[7] & 0x01)
							{
								mp3_info->frame_num = ((int)str[8] << 24) | ((int)str[9] << 16) | ((int)str[10] << 8) | (int)str[11];
								wxj_log("Info头附带信息 -->> 总帧数 [ %d ] \n", mp3_info->frame_num);
								mp3_info->total_time = (int)(mp3_info->frame_num * ((float)mp3_info->sample_num / (float)mp3_info->sample_rate)); // 标准估算，每帧的播放时间:无论帧长是多少，每帧的播放时间都是26ms
							}
							else
							{
								mp3_info->frame_num = (int)(total_data_frame_size / mp3_info->frame_size);
								wxj_log("Info头不含总帧数 -->> 采用公式计算 [ %d ] \n", mp3_info->frame_num);
								mp3_info->total_time = (int)((mp3_info->frame_num - 20) * ((float)mp3_info->sample_num / (float)mp3_info->sample_rate)) + 1; // 经验估算，每帧的播放时间:无论帧长是多少，每帧的播放时间都是26ms
							}

							wxj_log("公式计算时间:%d 秒\n", mp3_info->total_time);
						}
						else
						{
							wxj_log("XING头 和 Info头 均不存在，开始检测 VBRI头<<<====================\n");

							XING_head_pos = cur_pos + i + 4;
							memset(str, 0, 32);
							lseek(fd, XING_head_pos, SEEK_SET);
							read(fd, str, 32);

							if (str[0] == 'V' && str[1] == 'B' && str[2] == 'R' && str[3] == 'I')
							{
								wxj_log("VBRI头存在, 编码方式:VBR（可变比特率）\n");

								mp3_info->frame_num = ((int)str[14] << 24) | ((int)str[15] << 16) | ((int)str[16] << 8) | (int)str[17];
								wxj_log("VBRI头附带信息 -->> 总帧数 [ %d ] \n", mp3_info->frame_num);

								mp3_info->total_time = (int)(mp3_info->frame_num * mp3_info->sample_num / (mp3_info->bitrate * 1000)); // 公式计算
							}
							else
							{
								wxj_log("XING头 和 Info头 和 VBRI头 均不存在， 编码方式:CBR（固定比特率）<<<====================\n");

								total_data_frame_size = file_size - (ID3V1_SIZE * (int)(mp3_info->ID3V1_flag & 0x01)) - id3v2_info.ID3V2_size;
								wxj_log("直接采用公式计算所有数据帧大小 [ %d ] byte\n", total_data_frame_size);

								// mp3_info->total_time = (int)(total_data_frame_size / (mp3_info->bitrate * 1000) * 8);//网上的公式，精度未知

								mp3_info->frame_num = (int)(total_data_frame_size / mp3_info->frame_size);
								wxj_log("直接采用公式计算总帧数 [ %d ] \n", mp3_info->frame_num);
								mp3_info->total_time = (int)((mp3_info->frame_num - 20) * ((float)mp3_info->sample_num / (float)mp3_info->sample_rate)) + 1; // 经验估算，通过部分测试，精度较高
							}
						}
					}
					else
					{
						wxj_log("This a Free Rate MP3 File!\n");
					}
					free(buffer);
					buffer = NULL;
					break;
				}
			}
		}
		if (buffer == NULL)
		{
			if (fd)
				close(fd);
			if (mp3_info->bitrate != 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	if (buffer != NULL)
		free(buffer);

	if (close(fd) < 0)
	{
		wxj_log("file close error !\n");
	}
	return false;
}

#include "layout_define.h"

static bool music_task_play_status = true;
static int curr_music_index = 0;
int music_index_get(void);
void music_index_set(int index);
int music_total_get(void);
media_type music_file_type_get(void);
char *cur_music_path_get(void);

mp3_info_t music_info;

int music_total_time_get(void)
{
	return music_info.total_time;
}

int music_frame_num_get(void)
{
	return music_info.frame_num;
}

static rom_bin_info info_next = rom_bin_info_get(ROM_RES_MEDIA_NEXT_UNFOCUS_PNG);
static rom_bin_info info1_next = rom_bin_info_get(ROM_RES_MEDIA_NEXT_FOCUS_PNG);
static rom_bin_info info_pause = rom_bin_info_get(ROM_RES_MEDIA_PLAY_UNFOCUS_PNG);
static rom_bin_info info1_pause = rom_bin_info_get(ROM_RES_MEDIA_PAUSE_FOCUS_PNG);
static rom_bin_info info_prev = rom_bin_info_get(ROM_RES_MEDIA_PREV_UNFOCUS_PNG);
static rom_bin_info info1_prev = rom_bin_info_get(ROM_RES_MEDIA_PREV_FOCUS_PNG);
static rom_bin_info info_sub = rom_bin_info_get(ROM_RES_MEDIA_DEL_UNFOCUS_PNG);
static rom_bin_info info1_sub = rom_bin_info_get(ROM_RES_MEDIA_DEL_FOCUS_PNG);
static rom_bin_info info_add = rom_bin_info_get(ROM_RES_MEDIA_ADD_UNFOCUS_PNG);
static rom_bin_info info1_add = rom_bin_info_get(ROM_RES_MEDIA_ADD_FOCUS_PNG);
static rom_bin_info info_volume = rom_bin_info_get(ROM_RES_MEDIA_VOLUME_PNG);
static rom_bin_info info_music = rom_bin_info_get(ROM_RES_MEDIA_MUSIC_PNG);

typedef enum media_music_module_list
{
	NEXT_MODULE,
	PAUSE_MODULE,
	PREV_MODULE,
	VOL_SUB_MODULE,
	VOL_ADD_MODULE,
	VOLUME_MODULE,
	CURR_TIME_MODULE,
	PROGRESS_BAR_MODULE,
	TOTAL_TIME_MODULE,
	MUSIC_NAME_MODULE,
	TOTAL_MODULE
} media_music_module_list;

#define MEDIA_MUSIC_MODULE_COORDINATE_INIT { \
	{656, 533, 35, 35},                      \
	{586, 533, 35, 35},                      \
	{515, 533, 35, 35},                      \
	{735, 533, 35, 35},                      \
	{848, 533, 35, 35},                      \
	{784, 533, 37, 35},                      \
};

static void custom_music_play_finsih_callback(void);

static void start_playing_music(char *music_name, audio_play_callback end);

static void media_music_back_btn_up(lv_obj_t *obj)
{
	goto_layout(pLAYOUT(music_list));
}

static void media_music_prev_btn_up(lv_obj_t *obj)
{

	bool music_mp3_puase(bool puase);
	switch (obj->obj_id)
	{
	case NEXT_MODULE:
		/* if(music_task_play_status) */
		{
			if (music_task_play_status == false)
			{
				music_task_play_status = true;
				lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), PAUSE_MODULE);
				lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info_pause);
			}

			music_index_set(music_index_get() + 1);
			media_info *info = media_info_get(music_file_type_get(), music_index_get());
			printf("custom_music_play index :%d       name:%s\n\r", music_index_get(), info->file_name);
			start_playing_music(info->file_name, custom_music_play_finsih_callback);
			curr_music_index = music_index_get();
		}
		break;
	case PAUSE_MODULE:

		if ((music_task_play_status = !music_task_play_status))
		{
			lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info_pause);
		}
		else
		{
			lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info1_pause);
		}
		music_mp3_puase(!music_task_play_status);
		break;
	case PREV_MODULE:
		/* if(music_task_play_status) */
		{
			if (music_task_play_status == false)
			{
				music_task_play_status = true;
				lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), PAUSE_MODULE);
				lv_imgbtn_set_src(obj, LV_BTN_STATE_RELEASED, &info_pause);
			}

			music_index_set(music_index_get() - 1);
			media_info *info = media_info_get(music_file_type_get(), music_index_get());
			printf("custom_music_play index :%d       name:%s\n\r", music_index_get(), info->file_name);
			start_playing_music(info->file_name, custom_music_play_finsih_callback);
			curr_music_index = music_index_get();
		}
		break;
	case VOL_SUB_MODULE:
		if (user_data_get()->scene.bg_music_vol > 1)
		{
			user_data_get()->scene.bg_music_vol--;
			lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), VOLUME_MODULE);
			if (label != NULL)
			{
				char volume[4] = {0};
				sprintf(volume, "%d", user_data_get()->scene.bg_music_vol);
				lv_label_set_text(label, volume);
				audio_play_volume_set((user_data_get()->scene.bg_music_vol));
			}
		}
		break;
	case VOL_ADD_MODULE:
		if (user_data_get()->scene.bg_music_vol < 10)
		{
			user_data_get()->scene.bg_music_vol++;
			lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), VOLUME_MODULE);
			if (label != NULL)
			{
				char volume[4] = {0};
				sprintf(volume, "%d", user_data_get()->scene.bg_music_vol);
				lv_label_set_text(label, volume);
				audio_play_volume_set((user_data_get()->scene.bg_music_vol));
			}
		}
		break;
	default:
		break;
	}
}
static void media_music_btn_create(Controls_location coordinate, media_music_module_list module, rom_bin_info *src_info, rom_bin_info *src_info1)
{
	static btn_data btn_data = btn_data_create(NULL, media_music_prev_btn_up, NULL);

	btn_data.obj_tone = false;
	lv_obj_t *imgbtn1 = lv_imgbtn_create(lv_scr_act(), NULL);
	lv_obj_set_pos(imgbtn1, coordinate.x, coordinate.y);
	lv_obj_set_size(imgbtn1, coordinate.width, coordinate.high);

	lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_RELEASED, src_info);
	lv_obj_set_id(imgbtn1, module);

	lv_imgbtn_set_src(imgbtn1, LV_BTN_STATE_PRESSED, src_info1);
	imgbtn1->user_data = &btn_data;
	imgbtn1->obj_id = module;

	lv_imgbtn_set_checkable(imgbtn1, true);
	btn_touch_event_listen(imgbtn1);
}

static bool custom_music_power = false;
static void custom_music_play_stop(void)
{
	custom_music_power = false;
}
static void custom_music_play_finsih_callback(void)
{
	printf("%s    curr_music_index:%d    music_index_get():%d     custom_music_power:%d\n", __func__, curr_music_index, music_index_get(), custom_music_power);
	if (custom_music_power == false || curr_music_index != music_index_get())
	{
		curr_music_index = music_index_get();
		return;
	}
	music_index_set(music_index_get() + 1);
	media_info *info = media_info_get(music_file_type_get(), music_index_get());
	printf("custom_music_play index :%d       name:%s\n\r", music_index_get(), info->file_name);
	start_playing_music(info->file_name, custom_music_play_finsih_callback);
	curr_music_index = music_index_get();
}

static void media_music_progress_bar_display(int cur, int total)
{
	lv_obj_t *obj = lv_obj_get_child_form_id(lv_scr_act(), PROGRESS_BAR_MODULE);
	if (obj != NULL)
	{
		// Debug("cur :%d======================================total:%d\n\r",cur,total);
		lv_bar_set_range(obj, 0, 1000);
		lv_bar_set_anim_time(obj, 30);
		lv_bar_set_value(obj, cur * 1000 / total, LV_ANIM_OFF);

		lv_obj_t *timer_label = lv_obj_get_child_form_id(lv_scr_act(), CURR_TIME_MODULE);
		if (timer_label != NULL)
		{
			int cur_time = (int)(cur * 10000 / total * music_total_time_get() / 10000);
			lv_label_set_text_fmt(timer_label, "%02d:%02d", cur_time / 60, cur_time % 60);
		}
	}
}

static void media_music_progress_bar_create(void)
{
	lv_obj_t *obj = lv_bar_create(lv_scr_act(), NULL);
	lv_obj_set_id(obj, PROGRESS_BAR_MODULE);

	lv_obj_set_pos(obj, 87, 543);
	lv_obj_set_size(obj, 296, 16);
	lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x797878));
	lv_obj_set_style_local_radius(obj, LV_BAR_PART_BG, LV_STATE_DEFAULT, 8);
	lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x3BD741));
	lv_obj_set_style_local_radius(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 8);

	lv_obj_t *timer_label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(timer_label, CURR_TIME_MODULE);
	lv_obj_align(timer_label, obj, LV_ALIGN_OUT_LEFT_MID, -20, 0);
	lv_obj_set_size(timer_label, 100, 24);
	lv_label_set_align(timer_label, LV_LABEL_ALIGN_LEFT);
	lv_label_set_text(timer_label, "00:00");

	lv_obj_t *time_label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(time_label, TOTAL_TIME_MODULE);
	lv_obj_align(time_label, obj, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_obj_set_size(time_label, 100, 24);
	lv_label_set_align(time_label, LV_LABEL_ALIGN_RIGHT);
	lv_label_set_text(time_label, "--:--");
}

static lv_task_t *layout_music_task = NULL;

extern int mp3_music_palying_frame_num(void);
static void layout_music_timer_task(lv_task_t *task_t)
{
	// printf("play curr :%d       total:%d\n\r", mp3_music_palying_frame_num(), music_frame_num_get());
	media_music_progress_bar_display(mp3_music_palying_frame_num(), music_frame_num_get());
	// lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 555);
	// if(img)
	// {
	// 	static int16_t angle = 0;
	// 	angle += 50 ;
	// 	if(angle > 3600) angle = 0;
	// 	lv_img_set_angle(img, angle);
	// }

	// if(music_cur_played_frame_num >= music_frame_num_get())
	// {
	// 	if(layout_music_task != NULL)
	// 	{
	// 		lv_task_del(layout_music_task);
	// 		layout_music_task = NULL;
	// 	}
	// 	custom_music_play_finsih_callback();
	// }
}
static void media_music_total_time_display(int total)
{
	lv_obj_t *time_label = lv_obj_get_child_form_id(lv_scr_act(), TOTAL_TIME_MODULE);
	if (time_label != NULL)
	{
		lv_label_set_text_fmt(time_label, "%02d:%02d", total / 60, total % 60);
	}
}

static void media_music_name_label_display(void)
{
	lv_obj_t *label = lv_obj_get_child_form_id(lv_scr_act(), MUSIC_NAME_MODULE);
	if (label != NULL)
	{
		media_info *info = media_info_get(music_file_type_get(), music_index_get());

		lv_label_set_text(label, info->file_name);
		lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);
	}
}

static void media_music_name_label_create(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(label, MUSIC_NAME_MODULE);
	lv_obj_set_size(label, 300, 36);
	media_music_name_label_display();
	// lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);
	lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
}

static void media_music_img_create(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 360, 141);
	lv_obj_set_size(img, 292, 292);
	lv_img_set_src(img, &info_music);
	lv_obj_set_id(img, 555);
}

static void media_music_volume_img_create(void)
{
	lv_obj_t *img = lv_img_create(lv_scr_act(), NULL);
	lv_obj_set_pos(img, 784, 543);
	lv_obj_set_size(img, 300, 36);
	lv_img_set_src(img, &info_volume);
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_obj_set_id(label, VOLUME_MODULE);
	lv_obj_set_pos(label, 820, 537);
	lv_obj_set_size(label, 14, 28);
	char volume[4] = {0};
	sprintf(volume, "%d", user_data_get()->scene.bg_music_vol);
	lv_label_set_text(label, volume);
}

static void start_playing_music(char *music_name, audio_play_callback end)
{
	audio_play_stop_set();
	if (layout_music_task != NULL)
	{
		lv_task_del(layout_music_task);
		layout_music_task = NULL;
	}

	char path[50];
	memset(path, 0, 50);
	sprintf(path, "%s%s", cur_music_path_get(), music_name);

	printf("play music :%d       name:%s\n\r", music_index_get(), music_name);

	if (music_info_get(path, &music_info) == false)
	{
		goto_layout(pLAYOUT(music_list));
		return;
	}

	printf("===============>>>time:%d  =================>>>frame:%d \n", music_total_time_get(), music_frame_num_get());

	media_music_total_time_display(music_total_time_get());

	custom_music_play(music_name, user_data_get()->scene.bg_music_vol, false, NULL, end);

	layout_music_task = lv_task_create(layout_music_timer_task, 50, LV_TASK_PRIO_MID, NULL);

	media_music_name_label_display();
}

char music_name[128] = {0};

lv_anim_t a;
static void LAYOUT_ENETER_FUNC(media_music)
{
	Debug("==============LAYOUT_ENETER_FUNC====>>>>%d\n\n\n", user_data_get()->other.screen_saver);
	custom_music_power = true;

	void music_bg_display(void);
	music_bg_display();
	Controls_location module_coordinate[] = MEDIA_MUSIC_MODULE_COORDINATE_INIT;
	standby_timer_close();
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	lv_obj_invalidate(lv_scr_act());
	media_music_btn_create(module_coordinate[NEXT_MODULE], NEXT_MODULE, &info_next, &info1_next);
	media_music_btn_create(module_coordinate[PAUSE_MODULE], PAUSE_MODULE, &info_pause, &info_pause);
	media_music_btn_create(module_coordinate[PREV_MODULE], PREV_MODULE, &info_prev, &info1_prev);
	media_music_btn_create(module_coordinate[VOL_SUB_MODULE], VOL_SUB_MODULE, &info_sub, &info1_sub);
	media_music_btn_create(module_coordinate[VOL_ADD_MODULE], VOL_ADD_MODULE, &info_add, &info1_add);
	media_music_img_create();
	media_music_volume_img_create();
	media_music_progress_bar_create();
	media_music_name_label_create();
	home_back_btn_create(media_music_back_btn_up, NULL);
	media_info *info = media_info_get(music_file_type_get(), music_index_get());
	start_playing_music(info->file_name, custom_music_play_finsih_callback);
	curr_music_index = music_index_get();

#ifdef MUSIC_ANIM
	lv_obj_t *img = lv_obj_get_child_form_id(lv_scr_act(), 555);
	if (img)
	{
		lv_anim_init(&a);

		/* 必选设置
		 *------------------*/

		/* 设置“动画制作”功能 */
		lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_img_set_angle);

		/* 设置“动画制作”功能 */
		lv_anim_set_var(&a, img);

		/* 动画时长[ms] */
		lv_anim_set_time(&a, 5000);

		/* 设置开始和结束值。例如。 0、150 */
		lv_anim_set_values(&a, 0, 3600);

		/* 开始动画之前的等待时间[ms] */
		lv_anim_set_delay(&a, 0);

		/* 设置路径（曲线）。默认为线性 */
		lv_anim_path_t path;
		lv_anim_path_init(&path);
		lv_anim_path_set_cb(&path, lv_anim_path_linear);

		lv_anim_set_path(&a, &path);

		/* 设置一个回调以在动画准备好时调用。 */

		// lv_anim_set_ready_cb(&a, ready_cb);

		/* 设置在动画开始时（延迟后）调用的回调。 */
		// lv_anim_set_start_cb(&a, start_cb);

		/* 重复次数。默认值为1。LV_ANIM_REPEAT_INFINITE用于无限重复 */
		lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);

		/* 应用动画效果
		 *------------------*/
		lv_anim_start(&a);
		// 	static int16_t angle = 0;
		// 	angle += 50 ;
		// 	if(angle > 3600) angle = 0;
		// 	lv_img_set_angle(img, angle);
	}
#endif
}

static void LAYOUT_QUIT_FUNC(media_music)
{
	Debug("================================\n\r");
	standby_timer_open(-1, NULL);
#ifdef MUSIC_ANIM
	lv_anim_del(&a, NULL);
#endif
	lv_obj_set_style_local_bg_opa(lv_scr_act(), LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	system_bg_enable_set(true);
	lv_obj_set_click(lv_scr_act(), false);
	custom_music_play_stop();
	audio_play_stop_set();

	// ak_sleep_ms(1000);
	video_raw_clear();
	void screen_force_refresh(void);
	screen_force_refresh();
	home_bg_display();

	if (layout_music_task != NULL)
	{
		lv_task_del(layout_music_task);
		layout_music_task = NULL;
	}
}

CREATE_LAYOUT(media_music);
