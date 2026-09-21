#ifndef GPIO_API_H
#define GPIO_API_H
#include "stdbool.h"

typedef enum
{
    GPIO_IN = 0,
    GPIO_OUT = 1
}GPIO_DIR;


typedef enum
{
    GPIO_LEVEL_LOW = 0x01,
    GPIO_LEVEL_HIGH = 0x02,
    GPIO_LEVEL_UNKNOWN = 0x03
}GPIO_LEVEL;

typedef enum
{
    ISR_RISING = 0, //上升沿触发
    ISR_FALLING = 1, //下降沿触发
    ISR_BOTH = 2 // 边沿触发
}GPIO_EDGE;


typedef struct
{
	int door1_call_pin;	
	char door1_trigger_level;
	
	int door2_call_pin;
	char door2_trigger_level;
	
	int hook_pin;
	
	int alarm_det_pin_group[8];
	char alarm_trigger_level_group[8];
	int alarm_det_pin_num;
}isr_gpio_info;


bool gpio_set(const int pin,GPIO_LEVEL level);

bool gpio_read(const int pin,GPIO_LEVEL* level);

bool gpio_edge(const int pin , GPIO_EDGE edge);

bool pwm_set(const int pwm_no,unsigned int duty_cycle,unsigned int period);

bool set_gpio_pull_enable(const int pin,int enable);

/********************************************************************
pin_group:
	需要监听的io口。
trigger_leave:
	触发方式:
		GPIO_LEVEL_LOW,GPIO_LEVEL_HIGH,或者GPIO_LEVEL_LOW|GPIO_LEVEL_HIGH
依次顺序：
	pin_group[0]: door1_call_pin
	pin_group[1]: door2_call_pin
	pin_group[2]: hook_pin
	pin_group[3]-[11]: alarm1-alarm8
********************************************************************/
bool gpio_isr_init(const isr_gpio_info* isr_info);

#endif
