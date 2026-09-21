#include "leo_gpio.h"
#include "gpio_api.h"
#include "leo_pwm.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "../../lvgl/src/lv_misc/lv_task.h"

/**
 * @description: 获取时间差
 * @param {timespec} *last_time 基准时间
 * @param {unsigned long long} interval 时间间隔，时间差值大于时间间隔，更新基准时间
 * @return {*}  时间差值
 */
unsigned long long diff_clock_time_ms(struct timespec *last_time, unsigned long long interval)
{
	struct timespec curr_time;
	unsigned long long diff;
	clock_gettime(CLOCK_MONOTONIC, &curr_time);
	diff = (curr_time.tv_sec - last_time->tv_sec) * 1000 + (curr_time.tv_nsec - last_time->tv_nsec) / 1000000;

	if (interval < diff)
		*last_time = curr_time;
	return diff;
}

#define BL_EN_GPIO 82

static unsigned int PwmSyclePeriodGroup[5] = {
#ifndef _10_IPS_SCREEN
	8107, 19214, 39321, 52428, 65536
#else
	13107, 26214, 39321, 52428, 65536
#endif
};

static bool backlight_open_status = false;
bool backlight_status_get(void)
{
	return backlight_open_status;
}
void backlight_open(bool enable, bool is_monitor, int brightness)
{
	// printf("%s,%d\n", __func__, __LINE__);
	backlight_open_status = enable;
	if (enable)
	{
		if (is_monitor)
		{
			// printf("========>>>>backlight_open %d\n\n\n\n\n",brightness - 1);
			bl_pwm_duty_set(0, PwmSyclePeriodGroup[brightness - 1], PwmSyclePeriodGroup[4]);
		}
		else
		{
			// printf("========>>>>backlight_open %d\n\n\n\n\n",1);
			bl_pwm_duty_set(0, PwmSyclePeriodGroup[1], PwmSyclePeriodGroup[4]);
		}
	}
	else
	{
		// printf("========>>>>backlight_open disable\n\n\n\n\n");
		bl_pwm_duty_set(0, 1, PwmSyclePeriodGroup[4]);
	}
#if 0
	gpio_set(BL_EN_GPIO, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
#endif
}

void PWM1_AVDD(void)
{

	pwm1_duty_set(0, 47000, 65535);

#if 0
	gpio_set(BL_EN_GPIO, enable == true?GPIO_LEVEL_HIGH:GPIO_LEVEL_LOW);
#endif
}

#define CHRIME_DET_PIN 38
#define CHRIME_EN_PIN 52
void chime_gpio_disable(void)
{
	gpio_set(CHRIME_EN_PIN, GPIO_LEVEL_LOW);
}
void chime_gpio_enable(void)
{
	gpio_set(CHRIME_EN_PIN, GPIO_LEVEL_HIGH);
}
void chime_gpio_detect(void)
{
	static GPIO_LEVEL prev_level = GPIO_LEVEL_HIGH;
	static bool detect_beating = false;
	GPIO_LEVEL level;

	gpio_read(CHRIME_DET_PIN, &level);
	// printf("CHRIME_DET_PIN LEVEL :%d\n\r",level);
	if (prev_level != level)
	{
		if (detect_beating == false)
		{
			detect_beating = true;
		}
		else
		{
			detect_beating = false;
			prev_level = level;
			extern bool is_audio_play_ing(void);
			if (level == GPIO_LEVEL_HIGH /* && is_audio_play_ing() == false */)
			{
				extern bool door_chime_detect_push(void);
				door_chime_detect_push();
				return;
			}
			else if (level == GPIO_LEVEL_LOW)
			{
				return;
			}
		}
	}
	else
		detect_beating = false;
}

#ifdef ALARM_DETECT_ENABLE
#define SENSE1_DET_PIN 56
#define SENSE2_EN_PIN 30
void alarm_gpio_detect(unsigned int pin)
{
	static GPIO_LEVEL prev_confirm_state[2] = {GPIO_LEVEL_HIGH, GPIO_LEVEL_HIGH};
	static GPIO_LEVEL prev_trigger_state[2] = {GPIO_LEVEL_UNKNOWN, GPIO_LEVEL_UNKNOWN};

	GPIO_LEVEL level;
	int pin_index = pin == SENSE1_DET_PIN ? 0 : 1;

	gpio_read(pin, &level);
	if (prev_confirm_state[pin_index] != level)
	{
		if (prev_trigger_state[pin_index] == GPIO_LEVEL_UNKNOWN)
		{
			prev_trigger_state[pin_index] = level;
		}
		else
		{
			prev_confirm_state[pin_index] = level;
			prev_trigger_state[pin_index] = GPIO_LEVEL_UNKNOWN;

			if (level == GPIO_LEVEL_LOW)
			{
				prev_confirm_state[pin_index] = level;
				extern void alarm_sound_play(int volume);
				alarm_sound_play(8);
				extern void alarm_record_add(int sense);
				alarm_record_add(pin == SENSE1_DET_PIN ? 1 : 2);
				extern bool alarm_detect_push(void);
				alarm_detect_push();
				printf("SENSE%d trigger alarm!!!!\n\n", pin == SENSE1_DET_PIN ? 1 : 2);
			}
		}
	}
	else
	{
		prev_trigger_state[pin_index] = GPIO_LEVEL_UNKNOWN;
	}
}
#endif

#define SARADC_TALK_LED 53
#define SARADC_UNLOCK_LED 54

void talk_led_gpio_control(bool en)
{
}

void unlock_led_gpio_control(bool en)
{
}

#define MECHANICAL_KEY_PIN 55
#ifdef HOME_MECHANICAL_KEY
void mechanical_key_detect(void)
{
#ifdef SARADC_DETECT
	static int saradc_enable = -1;

	if (saradc_enable == -1)
	{
		FILE *file = fopen("/proc/device-tree/soc/saradc@08000000/status", "r");
		if (file == NULL)
		{
			perror("fopen /proc/device-tree/soc/saradc@08000000/status");
			// saradc_enable = 0;
			return;
		}

		char buffer[32];
		if (fgets(buffer, sizeof(buffer), file) != NULL)
		{
			buffer[strcspn(buffer, "\n")] = '\0'; // 去除换行符
			if (strcmp(buffer, "okay") == 0)
			{
				saradc_enable = 1;
				printf("saradc = okay\n");
			}
			else
			{
				saradc_enable = 0;
				printf("saradc != okay\n");
			}
		}
		else
		{
			perror("fgets");
			fclose(file);
			return;
		}
	}

	if (saradc_enable == 1)
	{
		char temp[64] = {0};
		static int prev_value = 2999;
		static int flag = -1;
		int value = 2999;
		// printf("==============>>%d\n\n\n", __LINE__);
		/***** 将字符串转换为10进制的数值 *****/

		// printf("==============>>%d\n\n\n",value);
		FILE *fp = NULL;

		if ((fp = popen("cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw", "r")) == NULL)
			return;

		fseek(fp, 0, SEEK_SET);
		memset(temp, 0, sizeof(temp));
		if (fgets(temp, sizeof(temp), fp) > 0)
		{
			temp[strlen(temp) - 1] = '\0';
			sscanf(temp, "%d", &value);
			// printf("==================key value:%d \n", value);
		}
		pclose(fp);

		extern void adc_key_event_push(unsigned long arg1, unsigned long arg2);
		if (value < 500 && prev_value < 500)
		{
			if (flag == -1)
			{
				flag = 0;
				static struct timespec time;
				if (diff_clock_time_ms(&time, 500) > 500)
					adc_key_event_push(2, 0);
			}
		}
		else if (abs(prev_value - value) < 100)
		{
			flag = -1;
		}
		prev_value = value;
	}
#endif

	static GPIO_LEVEL prev_level = GPIO_LEVEL_HIGH;
	static bool detect_beating = false;
	GPIO_LEVEL level;

	gpio_read(MECHANICAL_KEY_PIN, &level);
	if (prev_level != level)
	{
		if (detect_beating == false)
		{
			detect_beating = true;
		}
		else
		{
			detect_beating = false;
			prev_level = level;
			if (level == GPIO_LEVEL_LOW)
			{
				prev_level = level;
				extern bool mechanical_key_detect_push(void);
				mechanical_key_detect_push();
				printf("MECHANICAL_KEY PRESS TRIGGER !!!!\n\n");
			}
		}
	}
	else
		detect_beating = false;
}
#endif

void hardware_detect_task(lv_task_t *task_t)
{
	chime_gpio_detect();
#ifdef ALARM_DETECT_ENABLE
	alarm_gpio_detect(SENSE1_DET_PIN);
	alarm_gpio_detect(SENSE2_EN_PIN);
#endif

#ifdef HOME_MECHANICAL_KEY
	void mechanical_key_detect(void);
	mechanical_key_detect();
#endif
	return;
}

#define UBLLOCK 51
void unlock_gpio_set(bool enable)
{
	gpio_set(UBLLOCK, enable == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

#define MSG_LED_PIN 29
void msg_gpio_set(bool enable)
{
	gpio_set(MSG_LED_PIN, enable == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
	// printf("MSG_LED_PIN %s\n",enable ? "ON":"OFF");
}

#define AMP_ON_GPIO 37
void speak_enable_set(bool enable)
{
	static int en = -1;
	if (en != enable)
	{
		en = enable;
		// printf("%s %s\n", __func__, en ? "open" : "close");
		gpio_set(AMP_ON_GPIO, enable == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
	}
}

int check_speak_static(void) // 高电平：功放开
{
	GPIO_LEVEL level;
	gpio_read(AMP_ON_GPIO, &level);
	return (level == GPIO_LEVEL_HIGH ? false : true);
}
