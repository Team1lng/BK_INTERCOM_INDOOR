#ifndef LEO_PWM_H
#define LEO_PWM_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int bl_pwm_duty_set(const int pwm,unsigned int duty_cycle,unsigned int period);
int pwm1_duty_set(const int pwm,unsigned int duty_cycle,unsigned int period);
#endif
