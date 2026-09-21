/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-07-04 10:33:17
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-12-04 08:38:59
 * @FilePath: /two-wire-indoor/src/api/queue/ring_buffer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include "stdbool.h"

#include "ak_thread.h"

typedef struct
{
	char *head;
	char *tail;

	char *r_addr;
	char *w_addr;

	int cache_len;
	int ring_len;

	ak_mutex_t *mutex;

	ak_cond_t cond;
} ring_buffer;

bool ring_buffer_init(ring_buffer *ring, int size, ak_mutex_t *mutex);

bool ring_buffer_write(ring_buffer *ring, char *data, int size);

int ring_buffer_read(ring_buffer *ring, char *data, int size);

bool ring_buffer_release(ring_buffer *ring);

#endif
