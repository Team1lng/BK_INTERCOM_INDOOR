/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-04 10:33:18
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-11-28 17:25:50
 * @FilePath: /two-wire-indoor/src/tuya_batch_production_sdk/xls/tuya_uuid_and_key.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef TUYA_UUID_AND_KEY_H_
#define TUYA_UUID_AND_KEY_H_
#include <stdbool.h>

typedef struct
{
	char tuya_uuid[25];
	char tuya_key[32];
	int index;
	bool used;
	bool lock_id;
} tuya_conf_info;

typedef enum
{
	TUYA_FILE_NOT,
	TUYA_ID_NO_FIND,
	TUYA_FILE_FORMAT_ERROR,
	TUYA_FILE_RW_ERROR,
	TUYA_ETC_BACKUP_ERROR,
	TUYA_FILE_INDEX_NOT_EXIST,
	TUYA_ACTION_OK,
} TUYA_FILE_ERROR;

bool tuya_key_xls_exist_check(void);
bool tuya_uuid_etc_exist_check(void);
/***
**   日期:2022-06-30 09:48:37
**   修改者: link.wu
**   函数作用：从.xls中读取信息并修改.conf文件，获取新涂鸦信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_uuid_and_key_get(int index, char *uuid, char *key);
/***
**   日期:2022-06-28 16:25:56
**   修改者: link.wu
**   函数作用：判断是否有涂鸦uuid文档并获取信息
**   参数说明:
***/
TUYA_FILE_ERROR tuya_key_and_uuid_check(char *uuid, char *key, int *id_index);
/***
**   日期:2022-05-27 09:48:37
**   作者: leo.liu
**   函数作用：从xls中读取读取指定的文档
**   参数说明:
***/
TUYA_FILE_ERROR tuya_uuid_and_key_swtch(char *uuid, char *key, int *id_index);

bool tuya_conf_uuid_etc_read(tuya_conf_info *info_etc);

bool tuya_uuid_and_key_copy_create(char *uuid, char *key, int *id_index);
#endif