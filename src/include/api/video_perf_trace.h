#ifndef _VIDEO_PERF_TRACE_H_
#define _VIDEO_PERF_TRACE_H_

#include <stdbool.h>
#include <string.h>

typedef struct
{
	unsigned long calls;
	unsigned long successes;
	unsigned long failures;
	unsigned long zero_progress;
	unsigned long slow_calls;
	unsigned long busy_loops;
	unsigned long long bytes;
	unsigned long long total_ms;
	unsigned long long max_ms;
} video_perf_stats;

static inline void video_perf_stats_reset(video_perf_stats *stats)
{
	memset(stats, 0, sizeof(*stats));
}

static inline void video_perf_stats_record(video_perf_stats *stats,
										 unsigned int bytes,
										 unsigned long long elapsed_ms,
										 int result,
										 bool zero_progress,
										 unsigned int busy_loops,
										 unsigned int slow_threshold_ms)
{
	stats->calls++;
	stats->bytes += bytes;
	stats->total_ms += elapsed_ms;
	stats->busy_loops += busy_loops;

	if (elapsed_ms > stats->max_ms)
	{
		stats->max_ms = elapsed_ms;
	}
	if (elapsed_ms >= slow_threshold_ms)
	{
		stats->slow_calls++;
	}
	if (result == 0)
	{
		stats->successes++;
	}
	else
	{
		stats->failures++;
	}
	if (zero_progress)
	{
		stats->zero_progress++;
	}
}

#endif
