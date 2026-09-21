#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define Debug (printf("\n\033[0;32;40m[***%s***]:%u\033[0m \t", __PRETTY_FUNCTION__, __LINE__), printf)

#define SDMMC_DEVICE_NAME "mmcblk0"
#define SDMMC_PART_VDP_NAME "mmcblk0p1"
#define SDMMC_PART_TUYA_NAME "mmcblk0p2"

#define SD_TUYA_PATH "/mnt/tuya"
#define SD_BASE_PATH "/mnt/tf"
#define SDMMC_DEVICE_PATH "/dev"

static uint64_t block0_sector = 0, p1_sector = 0, p2_sector = 0, sector_size = 0, other_sector = 0;

/*************************************************************************
 * @brief  获取sd卡相关信息
 * @date   2023-03-20 16:19
 * @author xiaole
 * @return true/false  获取成功/失败
 * @param
 * @note
 **************************************************************************/
static bool _sdcard_info_get(void)
{
	if (access(SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME, F_OK) != 0)
	{
		Debug("%s not exists\n", SDMMC_DEVICE_NAME);
		return false;
	}

	char buffer[256] = {0};
	sprintf(buffer, "fdisk -l " SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME);
	Debug("%s\n", buffer);

	FILE *fp = NULL;
	if ((fp = popen(buffer, "r")) == NULL)
		return false;

	memset(buffer, 0, sizeof(buffer));
	while (fgets(buffer, sizeof(buffer), fp))
	{

		/* 获取扇区数量 */
		if (strstr(buffer, "Disk /dev/mmcblk0:") != NULL) /* Disk /dev/mmcblk0: 7500 MB, 7864320000 bytes, 15360000 sectors */
		{
			char *p_sectors_str = strrchr(buffer, ',') + 1; /* 查找第二2逗号 */
			sscanf(p_sectors_str, "%llu sectors", &block0_sector);
		}

		/* 获取扇区的大小 */
		if (strstr(buffer, "Units: sectors of") != NULL) /* Units: sectors of 1 * 512 = 512 bytes */
		{
			uint64_t a, b;
			sscanf(buffer, "Units: sectors of %llu * %llu = %llu bytes", &a, &sector_size, &b);
		}

		if (strstr(buffer, "/dev/mmcblk0p1") != NULL) /* 存在p1的分区 */
		{
			char str[8][32];
			for (int i = 0; i < 8; i++)
			{
				memset(str[i], 0, 32);
			}

			/* 获取p1分区的扇区数量 */
			sscanf(buffer, "%s\t%s\t%s\t%s\t%s\t%llu\t%s\t%s", str[0], str[1], str[2], str[3], str[4], &p1_sector, str[6], str[7]);
		}
		else if (strstr(buffer, "/dev/mmcblk0p2") != NULL) /* 存在p2分区 */
		{
			char str[8][32];
			for (int i = 0; i < 8; i++)
			{
				memset(str[i], 0, 32);
			}

			/* 获取p2分区的扇区数量 */
			sscanf(buffer, "%s\t%s\t%s\t%s\t%s\t%llu\t%s\t%s", str[0], str[1], str[2], str[3], str[4], &p2_sector, str[6], str[7]);
		}
		else if (strstr(buffer, "/dev/mmcblk0p") != NULL)
		{
			Debug("============find other part===============\n");
			other_sector = 1;
		}

		memset(buffer, 0, sizeof(buffer));
	}
	pclose(fp);

	return true;
}

/*************************************************************************
 * @brief  SD卡分区是否符合设定
 * @date   2023-03-20 19:57
 * @author xiaole
 **************************************************************************/
bool _sdcard_info_check_valid(void)
{
	block0_sector = 0, p1_sector = 0, p2_sector = 0, sector_size = 0, other_sector = 0;
	_sdcard_info_get();
	Debug("disk info block0_sector:[%llu], p1_sector:[%llu], p2_sector:[%llu], sector_size:[%llu]\n", block0_sector, p1_sector, p2_sector, sector_size);

	// uint64_t p1_start = 100 * 1024 * 1024 / 512;
	// uint64_t valid_sector = block0_sector - p1_start;

#ifdef SDCARD_PARTITION
	if (other_sector || p1_sector == 0 || p2_sector == 0 /* || abs(valid_sector / 3 - p1_sector) > 512 || abs(valid_sector * 2 / 3 - p2_sector) > 512 */)
	{
		Debug("_sdcard_info_check_valid : false\n");
		return false;
	}
#else
	if (p1_sector != 0 || p2_sector != 0 || other_sector)
	{
		Debug("_sdcard_info_check_valid : false\n");
		return false;
	}
#endif
	Debug("_sdcard_info_check_valid : true\n");
	return true;
}

/*************************************************************************
 * @brief  卸载/mnt 路径
 * @date   2023-03-20 16:21
 * @author xiaole
 * @return
 * @param
 * @note
 **************************************************************************/
bool _umount_mnt(void)
{
#if 1
	/* 卸载路径 */

#include <dirent.h>
	struct dirent *pdirent;
	DIR *d_info = opendir("/mnt");
	if (d_info)
	{
		while ((pdirent = readdir(d_info)) != NULL)
		{
			if ((strcmp(pdirent->d_name, ".") == 0) || (strcmp(pdirent->d_name, "..") == 0) || (strcmp(pdirent->d_name, "nfs") == 0))
				continue;

			if ((pdirent->d_type & DT_DIR))
			{

				char buf[24] = {0};
				sprintf(buf, "umount -l /mnt/%s", pdirent->d_name);
				Debug("%s\n", buf);
				system(buf);
				// system("mount");
			}
		}
	}

	closedir(d_info);
#else
	system("umount " SD_BASE_PATH);
	system("umount " SD_TUYA_PATH);
	system("umount /mnt/.upgrade");
#endif

	return true;
}

/*************************************************************************
 * @brief  格式化SD卡
 * @date   2023-03-20 16:24
 * @author xiaole
 * @return
 * @param
 * @note
 **************************************************************************/
bool _sdcard_format(int type)
{
	system("sync");
	if (type == 0)
	{
		if (access(SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME, F_OK) == 0)
		{
			Debug("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME " -n MeteC0 \n");
			system("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME " -n MeteC0 ");
		}
	}
	else if (type == 1)
	{
		if (access(SDMMC_DEVICE_PATH "/" SDMMC_PART_VDP_NAME, F_OK) == 0)
		{
			Debug("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_PART_VDP_NAME " -n MeteC1 \n");
			system("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_PART_VDP_NAME " -n MeteC1 ");
		}
	}
	else
	{
		if (access(SDMMC_DEVICE_PATH "/" SDMMC_PART_TUYA_NAME, F_OK) == 0)
		{
			Debug("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_PART_TUYA_NAME " -n MeteC2 \n");
			system("mkdosfs -F 32  " SDMMC_DEVICE_PATH "/" SDMMC_PART_TUYA_NAME " -n MeteC2 ");
		}
	}
	system("sync");
	return true;
}

/*************************************************************************
 * @brief  分区
 * @date   2023-03-20 16:30
 * @author xiaole
 * @return
 * @param
 * @note
 **************************************************************************/
bool _sdcard_repartition(void)
{
	uint64_t p1_start = 100 * 1024 * 1024 / 512;
	uint64_t total_sector = block0_sector - p1_start;
	uint64_t p1_end = total_sector / 2 + p1_start;

	if (access(SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME, F_OK) != 0)
	{
		Debug("%s not exists\n", SDMMC_DEVICE_NAME);
		return false;
	}

	FILE *fp1 = popen("fdisk " SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME, "wr");

	if (other_sector)
	{
		for (int i = 0; i < 10; i++)
		{
			char c = i + 48;
			fputc('d', fp1);
			fputc('\n', fp1);

			fputc(c, fp1);
			fputc('\n', fp1);
			Debug("d	delete a partition %c===============\n", c);
		}
	}
	else
	{
		if (p1_sector > 0) /* 删除p1分区 */
		{
			fputc('d', fp1);
			fputc('\n', fp1);

			fputc('1', fp1);
			fputc('\n', fp1);

			p1_sector = 0;
			Debug("d	delete a partition %d===============\n", 1);
		}

		if (p2_sector > 0) /* 删除p2分区 */
		{
			fputc('d', fp1);
			fputc('\n', fp1);
			fputc('\n', fp1);

			if (p1_sector == 0)
			{
				fputc('2', fp1);
				fputc('\n', fp1);
				Debug("d	delete a partition %d===============\n", 2);
			}
		}
	}

#ifdef SDCARD_PARTITION
	/* p1分区 */
	{

		char str0[128] = {0};
		char str1[128] = {0};
		sprintf(str0, "%llu", p1_start);
		sprintf(str1, "%llu", p1_end);

		Debug("add a new partition\n");
		Debug("primary partition (1)\n");
		Debug("First sector:%s\n", str0);
		Debug("Last sector:%s\n", str1);

		fputc('n', fp1);
		fputc('\n', fp1);

		fputc('p', fp1);
		fputc('\n', fp1);

		fputc('1', fp1);
		fputc('\n', fp1);

		fputs(str0, fp1);
		fputc('\n', fp1);

		fputs(str1, fp1);
		fputc('\n', fp1);
	}

	/* p2分区 */
	{
		char str0[128] = {0};
		sprintf(str0, "%llu", p1_end + 1);
		Debug("n	add a new partition\n");
		Debug("primary partition (2)\n");
		Debug("First sector:%s\n", str0);

		fputc('n', fp1);
		fputc('\n', fp1);

		fputc('p', fp1);
		fputc('\n', fp1);

		fputc('2', fp1);
		fputc('\n', fp1);

		fputs(str0, fp1);
		fputc('\n', fp1);

		fputc('\n', fp1);
		fputc('\n', fp1);
	}
#else
	/* 0分区 */
	{
		Debug("n	add a new partition\n");
		Debug("primary partition (0)\n");

		fputc('p', fp1);
		fputc('\n', fp1);
		fputc('\n', fp1);
	}
#endif

	Debug("w	write table to disk and exit\n");
	fputc('w', fp1);
	fputc('\n', fp1);

	pclose(fp1);

	system("sync");
	return true;
}

/*************************************************************************
 * @brief  重新加载mmc设备
 * @date   2023-03-20 16:44
 * @author xiaole
 * @return
 * @param
 * @note
 **************************************************************************/
static bool _mmc_reload(void)
{
	/* 两条指令之间需要延迟， 不然不能被检测到 removed/insert */

	usleep(500 * 1000);
	Debug("echo 0 > /sys/mmc_en/mmc1_card_pwr_en\n");
	system("echo 0   > /sys/mmc_en/mmc1_card_pwr_en");

	usleep(500 * 1000);
	Debug("echo 1 > /sys/mmc_en/mmc1_card_pwr_en\n");
	system("echo 1   > /sys/mmc_en/mmc1_card_pwr_en");
	return true;
}

/*************************************************************************
 * @brief  SD卡重新分区， 分区完后需重新插入SD卡
 * @date   2023-03-20 14:42
 * @return true 成功 false 失败/已分区
 * @note   由于涂鸦app回放与室内机本地回放这两个存储空间难以控制其大小，
 * 		   所以将sd卡重新分区，
 * 		   TUYA回放 占1/2
 * 		   剩余空间做其他用处
 *
 * 流程如下：
 * [1] 获取扇区数量 扇区大小
 * [2] 是否已按规则分区
 * [3] 卸载挂载点
 * [4] 重新分区
 * [5] 格式化分区
 **************************************************************************/
bool _sd_card_repartition_main(void)
{
	if (access(SDMMC_DEVICE_PATH "/" SDMMC_DEVICE_NAME, F_OK) != 0)
	{
		Debug("%s not exists\n", SDMMC_DEVICE_NAME);
		return false;
	}

	if (_sdcard_info_check_valid() == false)
	{
		Debug("sdcard repart is invalid\n");
		_umount_mnt();

		_sdcard_format(0);

		_sdcard_repartition();

		_mmc_reload();

		usleep(1000 * 1000 * 1); /* wait sdcard insert */
		_sdcard_format(1);
#ifdef SDCARD_PARTITION
		_sdcard_format(2);
#endif

		block0_sector = 0, p1_sector = 0, p2_sector = 0, sector_size = 0, other_sector = 0;
		return true;
	}

	return false;
}