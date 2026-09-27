#ifndef WIFI_SCAN_POLICY_H
#define WIFI_SCAN_POLICY_H

#include <stdbool.h>

static inline bool wifi_scan_try_begin(volatile bool *running)
{
	if (*running)
	{
		return false;
	}

	*running = true;
	return true;
}

static inline void wifi_scan_finish(volatile bool *running)
{
	*running = false;
}

#endif
