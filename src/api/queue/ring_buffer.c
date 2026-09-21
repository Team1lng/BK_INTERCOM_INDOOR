#include "ring_buffer.h"
#include "ak_mem.h"
#include "memory.h"

bool ring_buffer_init(ring_buffer *ring, int size, ak_mutex_t *mutex)
{
	if (ring->head != NULL)
	{
		ak_mem_free(ring->head);
	}
	ring->head = (char *)ak_mem_alloc(MODULE_ID_MEMORY, size);
	ring->r_addr = ring->w_addr = ring->head;

	ring->tail = ring->head + size;

	ring->cache_len = 0;
	ring->ring_len = size;
	ring->mutex = mutex;
	ak_thread_cond_init(&ring->cond);
	// ak_thread_mutex_init(&ring->mutex,NULL);
	return true;
}

bool ring_buffer_write(ring_buffer *ring, char *data, int size)
{
	ak_thread_mutex_lock(ring->mutex);
	if ((ring == NULL) || (ring->head == NULL) || (data == NULL))
	{
		printf("ring buffer point is null write\n");
		ak_thread_mutex_unlock(ring->mutex);
		return false;
	}
	if (size > ring->ring_len)
	{
		printf("ring buffer space low \n");
		ak_thread_mutex_unlock(ring->mutex);
		return false;
	}

	if ((ring->w_addr + size) > ring->tail)
	{
		int buf1_len = ring->tail - ring->w_addr;
		int buf2_len = size - buf1_len;
		memcpy(ring->w_addr, data, buf1_len);
		memcpy(ring->head, data + buf1_len, buf2_len);
		ring->w_addr = ring->head + buf2_len;
	}
	else
	{
		memcpy(ring->w_addr, data, size);
		ring->w_addr += size;
	}

	if ((ring->cache_len + size) > ring->ring_len)
	{
		int move_len = ring->cache_len + size - ring->ring_len;
		if ((ring->r_addr + move_len) > ring->tail)
		{
			int len1 = ring->tail - ring->r_addr;
			int len2 = move_len - len1;
			ring->r_addr = ring->head + len2;
		}
		else
		{
			ring->r_addr = ring->r_addr + move_len;
		}
		ring->cache_len = ring->ring_len;
	}
	else
	{
		ring->cache_len += size;
	}
	ak_thread_mutex_unlock(ring->mutex);
	ak_thread_cond_broadcast(&ring->cond);
	return true;
}

static void ring_buff_timewati(ring_buffer *ring, int ms)
{
	ak_thread_mutex_lock(ring->mutex);
	struct timespec timeout;
	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_nsec += ms * 1000 * 1000; // 将等待时间转换为纳秒

	// 处理纳秒溢出
	if (timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_sec += 1;
		timeout.tv_nsec -= 1000000000;
	}

	ak_thread_cond_timedwait(&ring->cond, ring->mutex, &timeout);
	ak_thread_mutex_unlock(ring->mutex);
}

int ring_buffer_read(ring_buffer *ring, char *data, int size)
{
	ring_buff_timewati(ring, 10);
	ak_thread_mutex_lock(ring->mutex);
	if ((ring == NULL) || (ring->head == NULL) || (data == NULL))
	{
		printf("ring buffer point is null read\n");
		ak_thread_mutex_unlock(ring->mutex);
		return -1;
	}
	if (ring->cache_len <= 0)
	{
		ak_thread_mutex_unlock(ring->mutex);
		return 0;
	}

	if (size > ring->cache_len)
	{
		size = ring->cache_len;
	}
/* ***************************************************** */
#if 0
	data = ring->r_addr;

	if((ring->r_addr + *size) > ring->tail)
	{
		*size = ring->tail - ring->r_addr;
		ring->r_addr =  ring->head;
	}
	else
	{
		ring->r_addr += *size;
	}

#else
	/* ***************************************************** */

	if ((ring->r_addr + size) > ring->tail)
	{
		int len1 = ring->tail - ring->r_addr;
		int len2 = size - len1;
		memcpy(data, ring->r_addr, len1);
		memcpy(data + len1, ring->head, len2);
		ring->r_addr = ring->head + len2;
	}
	else
	{
		memcpy(data, ring->r_addr, size);
		ring->r_addr += size;
	}
#endif
	ring->cache_len -= size;
	ak_thread_mutex_unlock(ring->mutex);
	return size;
}

bool ring_buffer_release(ring_buffer *ring)
{
	ak_thread_mutex_lock(ring->mutex);
	if (ring->head != NULL)
	{
		ak_mem_free(ring->head);
		ring->head = NULL;
	}
	ak_thread_mutex_unlock(ring->mutex);
	return true;
}
