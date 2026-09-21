#include "math.h"
#include "stdlib.h"
#include <stdbool.h>
#include "leo_api.h"
static int standby_timeout = 0;

static unsigned long long standby_timer_start = 0;

static void(*standby_timeout_func)(void) = NULL;

static bool standby_open_status = false;

bool standby_timer_open(int timeout,void(*timeout_callback)(void))
{
	if(timeout > 0)
	{
		standby_timeout = timeout;
	}

	if(timeout_callback != NULL)
	{
		standby_timeout_func = timeout_callback;
	}

	standby_timer_start = get_sys_ms();
	
	standby_open_status = true;
	return true;
}


bool standby_timer_close(void)
{
	standby_open_status = false;
	return true;
}


bool standby_timer_check(void)
{
	if(standby_open_status == false)
	{
		return false;
	}

	unsigned long long standby_timer_end = get_sys_ms();
	if(abs(standby_timer_end - standby_timer_start) > standby_timeout)
	{
		if(standby_timeout_func != NULL)
		{
			standby_timeout_func();
		}
		standby_timer_start = get_sys_ms();		
		return true;
	}
	return false;
}

bool standby_timer_reset(void)
{
	if(standby_open_status == false)
	{
		return false;
	}
	standby_timer_start = get_sys_ms();
	return true;
}




