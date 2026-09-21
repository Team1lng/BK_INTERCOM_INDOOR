#include "tuya_uuid_and_key.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "unistd.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "iniparser.h"
// #include "user_common.h"

#include "xls.h"

/*****  文件名格式为:xxxx-xx-xx.conf *****/
#define TUYA_UUID_CONF_ETC_PATH USER_FILE_PATH "tuya_key/"
/*****  文件名格式为”xxxx-xx-xx.xls“ *****/
#define TUYA_UUID_AND_KEY_XLS_PATH "/mnt/tf/tuya_key/"
/*****  tuya配置文档的文件名 *****/
static char tuya_uuid_and_key_mnt_xls[128] = {0};
static char tuya_uuid_and_key_mnt_ini[128] = {0};
static char tuya_uuid_and_key_etc_conf[128] = {0};
/*****  标志tuya id已经注册 *****/
static bool is_tuya_uuid_and_kay_register = false;

/***
**   日期:2022-06-30 08:19:56
**   修改者: link.wu
**   函数作用：判断flash中涂鸦.conf文件是否存在
**   参数说明:
***/
bool tuya_uuid_etc_exist_check(void)
{
	struct dirent *pdirent;
	struct tm tm;
	int find_count = 0;
	is_tuya_uuid_and_kay_register = false;

	DIR *d_info = opendir(TUYA_UUID_CONF_ETC_PATH);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".conf")) && (strlen(pdirent->d_name) == 15))
			{
				printf("FIND ===========>>%s\n", pdirent->d_name);
				sscanf(pdirent->d_name, "%04d-%02d-%02d.conf", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
		}
	}
	else
	{
		printf("open dir %s fail\n\r", TUYA_UUID_CONF_ETC_PATH);
	}
	bool reslut = false;

	if (find_count > 0)
	{
		is_tuya_uuid_and_kay_register = true;
		reslut = true;
		if (find_count == 1)
		{
			memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
			sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
			printf("read tuya config:%s \n", tuya_uuid_and_key_etc_conf);
		}
	}
	if (d_info)
	{
		closedir(d_info);
	}
	printf("%s ==============================>>>%d\n\r", __func__, reslut);
	return reslut;
}
/***
**   日期:2022-06-30 08:19:56
**   修改者: link.wu
**   函数作用：判断SD卡中涂鸦.ini文件是否存在
**   参数说明:
***/
static bool tuya_uuid_mnt_exist_check(const char *dir)
{
	printf("%s ==============================>>>\n\r", __func__);
	struct dirent *pdirent;
	struct tm tm;
	int find_count = 0;
	is_tuya_uuid_and_kay_register = false;

	DIR *d_info = opendir(dir);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".ini")) && (strlen(pdirent->d_name) == 14))
			{
				printf("FIND ===========>>%s\n", pdirent->d_name);
				sscanf(pdirent->d_name, "%04d-%02d-%02d.ini", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
			else if (!(pdirent->d_type & DT_REG))
			{
				printf("pdirent->d_type error\n\r");
			}
			else if ((strstr(pdirent->d_name, ".ini")) == NULL)
			{
				printf("ini error\n\r");
			}
			else if ((strlen(pdirent->d_name) != 14))
			{
				printf("strlen pdirent->d_name :%d\n\r", strlen(pdirent->d_name));
			}
		}
	}
	else
	{
		printf("open dir %s fail\n\r", dir);
	}
	bool reslut = false;

	if (find_count > 0)
	{
		is_tuya_uuid_and_kay_register = true;
		reslut = true;
		if (find_count == 1)
		{
			memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
			sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);
			printf("read tuya config:%s \n", tuya_uuid_and_key_mnt_ini);
		}
	}
	if (d_info)
	{
		closedir(d_info);
	}
	return reslut;
}

/***
**   日期:2022-06-30 08:13:17
**   修改者: link.wu
**   函数作用：遍历SD卡中涂鸦.xls文件是否存在
**   参数说明:
***/
bool tuya_key_xls_exist_check(void)
{
	printf("%s ==============================>>>\n\r", __func__);
	// if (is_tuya_uuid_and_kay_register == true)
	// {
	// 	return true;
	// }
	struct tm tm;
	struct dirent *pdirent;
	int find_count = 0;
	DIR *d_info = opendir(TUYA_UUID_AND_KEY_XLS_PATH);
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((pdirent->d_type & DT_REG) && (strstr(pdirent->d_name, ".xls")) && (strlen(pdirent->d_name) == 14))
			{
				sscanf(pdirent->d_name, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
				if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
				{
					find_count++;
				}
			}
		}
	}
	else
	{
		printf("open dir %s fail\n\r", TUYA_UUID_AND_KEY_XLS_PATH);
	}
	bool reslut = false;
	if (find_count == 1)
	{
		reslut = true;
		memset(tuya_uuid_and_key_mnt_xls, 0, sizeof(tuya_uuid_and_key_mnt_xls));
		sprintf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", tm.tm_year, tm.tm_mon, tm.tm_mday);
		printf("find tuya xls%s%s \n", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	}
	if (d_info)
	{
		closedir(d_info);
	}
	return reslut;
}

/***
**   日期:2022-06-28 16:25:56
**   修改者: link.wu
**   函数作用：判断是否有涂鸦.xls文档并获取信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_key_and_uuid_check(char *uuid, char *key, int *id_index)
{
#if 0
	if (tuya_uuid_mnt_exist_check(TUYA_UUID_AND_KEY_XLS_PATH) == true)
	{
		return true;
	}
	return tuya_key_xls_exist_check() == true ? false : true;
#else
	if (tuya_key_xls_exist_check() == true)
	{
		if (tuya_uuid_mnt_exist_check(TUYA_UUID_AND_KEY_XLS_PATH) == false)
		{
			if (tuya_uuid_and_key_copy_create(uuid, key, id_index) == false)
				return TUYA_FILE_FORMAT_ERROR;
		}
		else
		{
			return (tuya_uuid_and_key_swtch(uuid, key, id_index));
		}
		return TUYA_ACTION_OK;
	}
	return TUYA_FILE_NOT;
#endif
}

/***
**   日期:2022-05-27 16:10:13
**   作者: leo.liu
**   函数作用：判断.xls头文件是否满足格式
**   参数说明:
***/
static bool tuya_xls_head_valid_check(xlsWorkSheet *sheet)
{
	printf("lastcol = %d \n", sheet->rows.lastcol);
	if (sheet->rows.lastcol < 1)
	{
		return false;
	}

	struct st_row_data *row = &sheet->rows.row[0];
	unsigned char *row_str = row->cells.cell[0].str;
	if ((row_str == NULL) || (strcmp((char *)row_str, "uuid") != 0))
	{
		fflush(stdout);
		return false;
	}
	row_str = row->cells.cell[1].str;
	if ((row_str == NULL) || (strcmp((char *)row_str, "key") != 0))
	{
		fflush(stdout);
		return false;
	}

	for(int i = 0;i < sheet->rows.lastrow;i ++)
	{
		if(sheet->rows.row[i].cells.cell[0].str[0] == '\0' || sheet->rows.row[i].cells.cell[1].str[0] == '\0')
		{
			sheet->rows.lastrow = i;
		}
	}
	printf("lastrow = %d \n", sheet->rows.lastrow);
	return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：获取.conf文件中涂鸦uuid信息
**   参数说明:
**                       info_etc 	.conf文件涂鸦数据
***/
bool tuya_conf_uuid_etc_read(tuya_conf_info *info_etc)
{
	char conf_path[128] = {0};
	sprintf(conf_path, "%s%s", TUYA_UUID_CONF_ETC_PATH, tuya_uuid_and_key_etc_conf);

	FILE *fp = NULL;
	fp = fopen(conf_path, "r");
	if (fp == NULL)
	{
		printf("open %s fail\n\r", conf_path);
		return false;
	}
	fscanf(fp, "%d%s%s", &info_etc->index, info_etc->tuya_uuid, info_etc->tuya_key);
	printf("read %s index :%d uuid :%s	kye :%s\n\r", conf_path, info_etc->index, info_etc->tuya_uuid, info_etc->tuya_key);
	fclose(fp);

	if (strlen(info_etc->tuya_uuid) == 0 || strlen(info_etc->tuya_key) == 0)
	{
		remove(conf_path);
		return false;
	}
	return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：创建涂鸦.conf副本或添加涂鸦.conf文件信息
**   参数说明:
							info_mnt  .ini文件涂鸦数据
							info_etc	.conf文件涂鸦数据
***/
static int tuya_conf_uuid_etc_create(tuya_conf_info *info_mnt, tuya_conf_info *info_etc)
{
	struct tm tm;
	char conf_path[128] = {0};
	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	// printf("======================================>%d\n",__LINE__);
	if (tuya_uuid_etc_exist_check() == true)
	{
		// printf("======================================>%d\n",__LINE__);
		char temp_conf[128] = {0};
		memset(temp_conf, 0, sizeof(temp_conf));
		sprintf(temp_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
		if (strcmp(tuya_uuid_and_key_etc_conf, temp_conf) != 0)
		{
			printf("rm -rf " USER_FILE_PATH "tuya_key/*.conf\n");
			system("rm -rf " USER_FILE_PATH "tuya_key/*.conf");
		}
	}
	memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
	sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);
	sprintf(conf_path, "%s%s", TUYA_UUID_CONF_ETC_PATH, tuya_uuid_and_key_etc_conf);

	FILE *fp = NULL;
	fp = fopen(conf_path, "w");
	if (fp == NULL)
	{
		printf("tuya_conf_uuid_etc %s fp NULL\n\r", conf_path);
		perror("打开文件test.txt失败啦");
		fclose(fp);
		return -1;
	}
	fprintf(fp, "%d %s %s\n", info_mnt->index, info_mnt->tuya_uuid, info_mnt->tuya_key);
	*info_etc = *info_mnt;
	printf("INFO_ETC.TUYA_UUID :%s\n\r", info_etc->tuya_uuid);
	fclose(fp);

	printf("%s,%d\n", __func__, __LINE__);
	system("sync");
	return 0;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：创建涂鸦.ini副本或添加SD涂鸦信息
**   参数说明:
							fp					涂鸦文档.ini文件句柄
							info_mnt   涂鸦数据
***/
static bool tuya_conf_uuid_mnt_create(FILE *fp, tuya_conf_info *info_mnt)
{
	if (fp == NULL)
	{
		printf("tuya_conf_uuid_mnt fp NULL\n\r");
		return false;
	}
	// printf("tuya_conf_uuid_mnt_create\n\r");
	fprintf(fp, "[%d]\n uuid = %s\n key = %s\n used = %s\n\n", info_mnt->index, info_mnt->tuya_uuid, info_mnt->tuya_key, info_mnt->used ? "true" : "false");

	return true;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：修改SD卡中涂鸦.ini副本数据参数
**   参数说明:
							ini_name  SD卡涂鸦.ini副本名
							info_mnt  涂鸦数据
***/
#include "errno.h"
#include <string.h>
int tuya_conf_uuid_mnt_set(dictionary *ini, char *ini_name, tuya_conf_info *info)
{
	printf("%s================%s===============>>>%d\n\r", __func__, ini_name, info->index);
	unsigned long long os_get_ms(void);
	unsigned long long x = os_get_ms();
	if (info->index != 0)
	{
		dictionary *ini_tmp;
		char section[10] = {0};
		char entry[10] = {0};
		char val[10] = {0};

		sprintf(val, "%s", info->used ? "true" : "false");
		sprintf(section, "%d", info->index);
		sprintf(entry, "%s:used", section);
		printf("%s================%s===============>>>%s\n\r", val, entry, section);
		/* Some temporary variables to hold query results */

		if (ini == NULL)
		{
			ini_tmp = iniparser_load(ini_name);
			if (ini_tmp == NULL)
			{
				fprintf(stderr, "cannot parse file: %s\n", ini_name);
				return -1;
			}
		}
		else
		{
			ini_tmp = ini;
		}
		// iniparser_dump(ini, stderr);
		iniparser_set(ini, entry, val);

		FILE *fp = NULL;
		int errNum;
		fp = fopen(ini_name, "w");
		if (fp == NULL)
		{
			printf("stone:fopen %s error!\n", ini_name);
			errNum = errno;
			printf("open fail errno = %d \n", errNum);
			if (ini == NULL)
			{
				iniparser_freedict(ini_tmp);
			}
			return -1;
		}

		x = os_get_ms();
		iniparser_dump_ini(ini_tmp, fp);
		printf("%s[%d]===============================>>>%llu\n\r", __func__, __LINE__, os_get_ms() - x);
		x = os_get_ms();
		fclose(fp);
		if (ini == NULL)
		{
			iniparser_freedict(ini_tmp);
		}
		system("sync");
	}
	return 0;
}

/***
**   日期:2022-06-30 16:10:13
**   作者: link.wu
**   函数作用：读取SD卡中涂鸦.ini文件数据参数
**   参数说明:
							ini  SD卡涂鸦.ini副本对象
							info_mnt  涂鸦数据
***/
int tuya_conf_uuid_mnt_read(dictionary *ini, tuya_conf_info *info_mnt)
{
	if (ini == NULL)
	{
		printf("cannot parse file:uuid_mnt\n");
		return -1;
	}

	if (info_mnt->index != 0)
	{
		char entry[10] = {0};
		sprintf(entry, "%d:used", info_mnt->index);
		info_mnt->used = iniparser_getboolean(ini, entry, 0);
		if (info_mnt->used == false)
		{
			bzero(entry, sizeof(entry));
			sprintf(entry, "%d:uuid", info_mnt->index);

			const char *uuid = iniparser_getstring(ini, entry, NULL);
			if (uuid == NULL)
			{
				printf("tuya_conf_uuid_mnt_read uuid fail ....\n\r");
				return -1;
			}
			else
			{
				sprintf(info_mnt->tuya_uuid, "%s", iniparser_getstring(ini, entry, NULL));
			}
			bzero(entry, sizeof(entry));
			sprintf(entry, "%d:key", info_mnt->index);
			const char *key = iniparser_getstring(ini, entry, NULL);
			if (key == NULL)
			{
				printf("tuya_conf_uuid_mnt_read key fail ....\n\r");
				return -1;
			}
			else
			{
				sprintf(info_mnt->tuya_key, "%s", iniparser_getstring(ini, entry, NULL));
			}
		}
	}

	return info_mnt->used;
}

/***
**   日期:2022-06-30 16:35:28
**   作者: link.wu
**   函数作用：读取SD卡中.ini文件中未使用的uuid
**   参数说明:
							ini  SD卡涂鸦文档副本对象
							info_mnt  涂鸦数据
***/
static bool tuya_uuid_ini_null_find(dictionary *ini, tuya_conf_info *info_mnt)
{
	if (tuya_conf_uuid_mnt_read(ini, info_mnt) == 0)
	{
		printf(" SD save uuid sd to flash succeed 1\n");
		return true;
	}
	return false;
}

/***
**   日期:2022-06-30 16:35:28
**   修改者: link.wu
**   函数作用：读取涂鸦xls文件信息保存至ini即conf副本中
**   参数说明:
							sheet    .xls文件数据接口
							fp           .ini副本句柄
							info_mnt	.ini涂鸦数据
							info_etc	  .conf 涂鸦数据
***/
static bool tuya_xls_uuid_key_read(const xlsWorkSheet *sheet, FILE *fp, tuya_conf_info *info_mnt, tuya_conf_info *info_etc)
{
	struct st_row_data *row = &sheet->rows.row[info_mnt->index];

	sprintf(info_mnt->tuya_uuid, "%s", row->cells.cell[0].str);
	if (row->cells.cell[0].str == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}

	if (row->cells.cell[0].str[0] == '\0')
	{
		return true;
	}
	// printf("%s+%d,%s\n\r", __func__, strlen(row->cells.cell[1].str), row->cells.cell[1].str);
	sprintf(info_mnt->tuya_key, "%s", row->cells.cell[1].str);
	if (row->cells.cell[1].str == NULL)
	{
		printf("read uuid is null \n");
		return false;
	}

	if (row->cells.cell[1].str[0] == '\0')
	{
		return true;
	}

	if (info_mnt->used == true)
	{
		if (tuya_conf_uuid_etc_create(info_mnt, info_etc) == -1)
		{
			return false;
		}
	}

	if (tuya_conf_uuid_mnt_create(fp, info_mnt) == false)
	{
		printf("SD save uuid sd to flash faild 2\n");
		return false;
	}

	return true;
}

#if 1
/***
**   日期:2022-06-30 15:00:00
**   修改者: link.wu
**   函数作用：从xls创建ini副本
**   参数说明:
***/
bool tuya_uuid_and_key_copy_create(char *uuid, char *key, int *id_index)
{
	printf("%s+========================%s   %s\n\r", __func__, uuid, key);
	bool reslut = true;
	/*****  已经有注册的uuid文档 *****/
	// if (is_tuya_uuid_and_kay_register == true)
	// {
	// 	return false;
	// }
	char xls_path[128] = {0};

	char conf_path[128] = {0};

	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return false;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet); // 解析XLS表

	/*****  判断头文件 *****/
	if (tuya_xls_head_valid_check(pxls_sheet) == false)
	{
		printf("xls head valid failed \n");
		reslut = false;
		goto finish;
	}

	/*****  保存指定的tuyaid *****/
	FILE *fp = NULL;
	struct tm tm;

	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_XLS_PATH, F_OK))
		{
			return false;
		}

		memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
		sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);
		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_ini);
		printf("open a+ %s\n", conf_path);
		fp = fopen(conf_path, "w");

		if (fp == NULL)
		{
			printf("open %s fail..............\n", conf_path);
			return -1;
		}
		tuya_conf_info info_mnt = {{0}, {0}, 0, false};
		tuya_conf_info info_etc = {{0}, {0}, 0, false};
		for (info_mnt.index = 1; info_mnt.index <= pxls_sheet->rows.lastrow; info_mnt.index++)
		{

			info_mnt.used = info_mnt.index == 1 ? true : false;

			// printf("%s+========================%d\n\r", __func__, __LINE__);
			if (tuya_xls_uuid_key_read(pxls_sheet, fp, &info_mnt, &info_etc) == false)
			{
				printf("xls save failed index:%d \n", info_mnt.index);
				reslut = false;
				fclose(fp);
				goto finish;
			}
			// printf("%s+========================%d %d\n\r", __func__, info_mnt.index, pxls_sheet->rows.lastrow);
		}

		printf("%s+========================%d\n\r", __func__, __LINE__);
		*id_index = info_etc.index;
		sprintf(uuid, info_etc.tuya_uuid, sizeof(info_etc.tuya_uuid));
		sprintf(key, info_etc.tuya_key, sizeof(info_etc.tuya_key));
		printf("info_etc.tuya_uuid :%s         uuid:%s\n\r", info_etc.tuya_uuid, uuid);
		printf("info_etc.tuya_key :%s           key:%s\n\r", info_etc.tuya_key, key);

		fclose(fp);

		tuya_conf_uuid_mnt_set(NULL, conf_path, &info_etc);
	}
finish:
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return reslut;
}

// #else

/***
**   日期:2022-06-30 09:48:37
**   修改者: link.wu
**   函数作用：从.conf中读取信息并修改.ini文件，获取新涂鸦信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_uuid_and_key_swtch(char *uuid, char *key, int *id_index)
{
	printf("%s+=======================11=\n\r", __func__);
	TUYA_FILE_ERROR reslut = TUYA_ID_NO_FIND;
	/*****  已经有注册的uuid文档 *****/
	char xls_path[128] = {0};

	char conf_path[128] = {0};

	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	printf("open %s+========================\n\r", xls_path);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return TUYA_FILE_RW_ERROR;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet); // 解析XLS表

	/*****  保存指定的tuyaid *****/
	struct tm tm;
	tuya_conf_info info_mnt = {{0}, {0}, 0, false};
	tuya_conf_info info_etc = {{0}, {0}, 0, false};
	dictionary *ini = NULL;

	/*****  判断头文件 *****/
	if (tuya_xls_head_valid_check(pxls_sheet) == false)
	{
		printf("xls head valid failed \n");
		reslut = false;
		goto exit;
	}
	
	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_XLS_PATH, F_OK) != 0)
		{
			goto exit;
		}

		memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
		sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);

		memset(tuya_uuid_and_key_etc_conf, 0, sizeof(tuya_uuid_and_key_etc_conf));
		sprintf(tuya_uuid_and_key_etc_conf, "%04d-%02d-%02d.conf", tm.tm_year, tm.tm_mon, tm.tm_mday);

		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_ini);
		// printf("open %s\n", conf_path);

		/* 读出原有UUID信息 */
		if (tuya_conf_uuid_etc_read(&info_etc) == false)
		{
			info_mnt.index = 1;
		}
		else
		{
			info_mnt.index = info_etc.index + 1;
		}
		ini = iniparser_load(conf_path);
		if (ini == NULL)
		{
			printf("iniparser_load %s fail\n", conf_path);
			reslut = TUYA_FILE_RW_ERROR;
			goto exit;
		}
		/* Some temporary variables to hold query results */
		// iniparser_dump(ini, stderr);
		for (; info_mnt.index <= pxls_sheet->rows.lastrow; info_mnt.index++) // 当前UUID索引查找至文件尾
		{

			if (tuya_uuid_ini_null_find(ini, &info_mnt) == true)
			{
				printf("tuya_uuid_ini_null_find succeed:%d ========%d\n", info_mnt.index, __LINE__);
				reslut = TUYA_ACTION_OK;
				goto find_finish;
			}
		}

		for (info_mnt.index = 1; info_mnt.index <= info_etc.index; info_mnt.index++) // 文件头查找至当前UUID索引
		{
			if (tuya_uuid_ini_null_find(ini, &info_mnt) == true)
			{
				printf("tuya_uuid_ini_null_find succeed:%d ========%d\n", info_mnt.index, __LINE__);
				reslut = TUYA_ACTION_OK;
				goto find_finish;
			}
		}

	find_finish:
		if (reslut == TUYA_ACTION_OK)
		{
			info_mnt.used = true;
			*id_index = info_mnt.index;
			if (tuya_conf_uuid_mnt_set(ini, conf_path, &info_mnt) == -1)
			{
				reslut = TUYA_FILE_RW_ERROR;
				goto exit;
			}

			info_etc.used = false;
			if (tuya_conf_uuid_mnt_set(ini, conf_path, &info_etc) == -1)
			{
				reslut = TUYA_FILE_RW_ERROR;
				goto exit;
			}

			if (tuya_conf_uuid_etc_create(&info_mnt, &info_mnt) == -1)
			{
				goto exit;
			}
			printf("%s,%d\n", __func__, __LINE__);
			sprintf(uuid, info_mnt.tuya_uuid, sizeof(info_mnt.tuya_uuid));
			sprintf(key, info_mnt.tuya_key, sizeof(info_mnt.tuya_key));
		}
	}
	printf("%s,%d\n", __func__, __LINE__);
exit:
	if (ini != NULL)
		iniparser_freedict(ini);

	printf("%s,%d\n", __func__, __LINE__);
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return reslut;
}

/***
**   日期:2022-06-30 09:48:37
**   修改者: link.wu
**   函数作用：从.xls中读取信息并修改.conf文件，获取新涂鸦信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_uuid_and_key_get(int index, char *uuid, char *key)
{
	printf("%s+=======================11=\n\r", __func__);
	TUYA_FILE_ERROR reslut = TUYA_ACTION_OK;
	char xls_path[128] = {0};

	char conf_path[128] = {0};

	sprintf(xls_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_xls);
	xlsWorkBook *pxls_xls = xls_open(xls_path, "UTF-8");
	if (pxls_xls == NULL)
	{
		printf("unable to open %s \n", xls_path);
		return TUYA_FILE_FORMAT_ERROR;
	}
	xlsWorkSheet *pxls_sheet = xls_getWorkSheet(pxls_xls, 0);
	xls_parseWorkSheet(pxls_sheet); // 解析XLS表

	/*****  判断头文件 *****/
	if (tuya_xls_head_valid_check(pxls_sheet) == false)
	{
		printf("xls head valid failed \n");
		reslut = TUYA_FILE_FORMAT_ERROR;
		goto finish;
	}

	/*****  保存指定的tuyaid *****/
	FILE *fp = NULL;
	struct tm tm;

	sscanf(tuya_uuid_and_key_mnt_xls, "%04d-%02d-%02d.xls", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
	if ((tm.tm_year > 2021) && (tm.tm_year < 2099) && (tm.tm_mon > 0) && (tm.tm_mon < 13) && (tm.tm_mday > 0) && (tm.tm_mday < 32))
	{
		if (access(TUYA_UUID_AND_KEY_XLS_PATH, F_OK))
		{
			return TUYA_FILE_NOT;
		}

		memset(tuya_uuid_and_key_mnt_ini, 0, sizeof(tuya_uuid_and_key_mnt_ini));
		sprintf(tuya_uuid_and_key_mnt_ini, "%04d-%02d-%02d.ini", tm.tm_year, tm.tm_mon, tm.tm_mday);
		sprintf(conf_path, "%s%s", TUYA_UUID_AND_KEY_XLS_PATH, tuya_uuid_and_key_mnt_ini);
		printf("open a+ %s\n", conf_path);
		fp = fopen(conf_path, "w");

		if (fp == NULL)
		{
			printf("open %s fail..............\n", conf_path);
			reslut = TUYA_FILE_RW_ERROR;
			goto finish;
		}

		if (index > pxls_sheet->rows.lastrow)
		{
			printf("index %d not exist..............\n", index);
			reslut = TUYA_FILE_INDEX_NOT_EXIST;
			goto error;
		}

		tuya_conf_info info_etc = {{0}, {0}, 0, false};
		struct st_row_data *row = &pxls_sheet->rows.row[index];

		info_etc.index = index;
		info_etc.used = true;
		sprintf(info_etc.tuya_uuid, "%s", row->cells.cell[0].str);
		if (row->cells.cell[0].str == NULL)
		{
			printf("read uuid is null \n");
			reslut = TUYA_ID_NO_FIND;
			goto error;
		}
		sprintf(info_etc.tuya_key, "%s", row->cells.cell[1].str);
		if (row->cells.cell[1].str == NULL)
		{
			printf("read key is null \n");
			reslut = TUYA_ID_NO_FIND;
			goto error;
		}

		if (tuya_conf_uuid_etc_create(&info_etc, &info_etc) == -1)
		{
			printf("tuya etc backup fail.... \n");
			reslut = TUYA_ETC_BACKUP_ERROR;
			goto error;
		}

		printf("%s+========================%d\n\r", __func__, __LINE__);
		sprintf(uuid, info_etc.tuya_uuid, sizeof(info_etc.tuya_uuid));
		sprintf(key, info_etc.tuya_key, sizeof(info_etc.tuya_key));
		printf("info_etc.tuya_uuid :%s         uuid:%s\n\r", info_etc.tuya_uuid, uuid);
		printf("info_etc.tuya_key :%s           key:%s\n\r", info_etc.tuya_key, key);

	error:
		fclose(fp);
	}
finish:
	xls_close_WS(pxls_sheet);
	xls_close_WB(pxls_xls);
	return reslut;
}
#endif
