#include "audio_input.h"
#include "ak_ai.h"
#include "string.h"
#include "ak_thread.h"
#include "queue.h"
#include "ak_mem.h"
#include "audio_output.h"
#include "leo_api.h"

#include"../../include/g711/g711_table.h"
#include"ak_mem.h"
#include <unistd.h>
#include <fcntl.h>
#define AUDIO_DECODE_QUEUE_MAX 8

bool audio_output_buffer_get(int *total, int *remain);

typedef struct
{
	unsigned char *data;
	int len;
} audio_decode_info;
typedef struct
{
	void *prev;
	void *next;

	audio_decode_info msg;
} audio_decode_queue;
static audio_decode_queue audio_decode_buffer[AUDIO_DECODE_QUEUE_MAX];
static queue_s audio_decode_queue_free;
static queue_s audio_decode_queue_head;
static ak_mutex_t audio_decode_queue_free_mutex;
static ak_mutex_t audio_decode_queue_head_mutex;
ak_mutex_t audio_output_open_mutex;

static audio_decode_queue *audio_decode_queue_node_new(unsigned char *data, int len)
{
	audio_decode_queue *node = NULL;
	ak_thread_mutex_lock(&audio_decode_queue_free_mutex);
	if (queue_empty(&audio_decode_queue_free) == 0)
	{
		node = (audio_decode_queue *)queue_delete_next(&audio_decode_queue_free);
		if (node->msg.data != NULL)
		{
			ak_mem_free(node->msg.data);
		}
		node->msg.data = ak_mem_alloc(MODULE_ID_AI, len);
		node->msg.len = len;
		memcpy(node->msg.data, data, len);
	}
	ak_thread_mutex_unlock(&audio_decode_queue_free_mutex);
	return node;
}

static void audio_decode_queue_node_del(audio_decode_queue *node)
{
	if (node != NULL)
	{
		if (node->msg.data != NULL)
		{
			ak_mem_free(node->msg.data);
			node->msg.data = NULL;
		}
		ak_thread_mutex_lock(&audio_decode_queue_free_mutex);
		queue_insert((queue_s *)node, &audio_decode_queue_free);
		ak_thread_mutex_unlock(&audio_decode_queue_free_mutex);
	}
}

static bool audio_decode_task_run = false;
static bool audio_decode_thread_run = false;
static bool audio_decode_ready = false;

void audio_decode_device_open(int vol,bool force)
{
	ak_thread_mutex_lock(&audio_output_open_mutex);
	extern bool is_audio_play_ing(void);
	if(is_audio_play_ing() == false || force)
	{
		printf("%s =============================================%d\n\r", __func__,vol);
		audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_16000,vol, 6);
	}
	ak_thread_mutex_unlock(&audio_output_open_mutex);
}
static void *audio_decode_task(void *arg)
{
	int *vol = (int* )arg;
	audio_decode_thread_run = true;
	alaw_pcm16_tableinit();
	bool audio_skip_frame = false;
	unsigned long long check_audio_buffer_timer = get_sys_ms();
	// int fb = -1;
	// fb = open("/mnt/nfs/write.pcm",O_CREAT|O_WRONLY);
	// int pcm_size = 0;
	while (audio_decode_task_run == true)
	{
		audio_decode_queue *node = NULL;
		ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
		if (queue_empty(&audio_decode_queue_head) == 0)
		{
			node = (audio_decode_queue *)queue_delete_next(&audio_decode_queue_head);
		}
		ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);

		if (node != NULL)
		{
			if ((audio_decode_ready == true) && (audio_skip_frame == false))
			{
				#if 1
				int pcm_buffer_size = node->msg.len *2;
				char *pcm_buffer = NULL;
				pcm_buffer = malloc(pcm_buffer_size);
				if(pcm_buffer == NULL)
				{
					printf("Error while allocating memory for write buffer.\n");
					audio_decode_queue_node_del(node);
					continue;
				}
				alaw_to_pcm16(node->msg.len,(const char *)node->msg.data,pcm_buffer);
				// if(pcm_size + pcm_buffer_size < 1024*300 && (fb != -1))
				// {
				// 	write(fb,pcm_buffer,pcm_buffer_size);
				// }
				// else if(fb != -1 && fb != -2)
				// {
				// 	printf("record audio pcm over!!!!!\n\r");
				// 	close(fb);
				// 	fb = -2;
				// }

				if (audio_output_write((unsigned char*)pcm_buffer, pcm_buffer_size, false,NULL) == false)
				{
					audio_decode_device_open(*vol,true);
				}
				free(pcm_buffer);
				pcm_buffer = NULL;
				#else
				// if(pcm_size + node->msg.len < 1024*300 && (fb != -1))
				// {
				// 	pcm_size += node->msg.len;
				// 	write(fb,node->msg.data,node->msg.len);
				// }
				// else if(fb != -1 && fb != -2)
				// {
				// 	printf("record audio pcm over!!!!!\n\r");
				// 	close(fb);
				// 	fb = -2;
				// }

				if (audio_output_write((unsigned char*)node->msg.data, node->msg.len, false,NULL) == false)
				{
					audio_decode_device_open();
				}
				#endif
			}
			audio_decode_queue_node_del(node);
		}
		else
		{
			unsigned long long ms = get_sys_ms();
			if ((audio_skip_frame == true) || ((ms - check_audio_buffer_timer) > 3000))
			{
				check_audio_buffer_timer = ms;

				int total, remain;
				if ((audio_output_buffer_get(&total, &remain) == true) && (remain > 10000))
				{
					audio_skip_frame = true;
				}
				else
				{
					audio_skip_frame = false;
				}
			}
			ak_sleep_ms(1);
		}
	}
	ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
	audio_decode_ready = false;
	while (queue_empty(&audio_decode_queue_head) == 0)
	{
		audio_decode_queue *node = (audio_decode_queue *)queue_delete_next(&audio_decode_queue_head);
		audio_decode_queue_node_del(node);
	}
	ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
	audio_decode_thread_run = false;
	ak_thread_exit();
	return NULL;
}

static bool audio_decode_wait_thread_quit(void)
{
	int timeout = 300;
	while (timeout--)
	{
		if (audio_decode_thread_run == false)
		{
			return true;
		}
		ak_sleep_ms(10);
	}
	return false;
}

static void audio_decode_queue_init(void)
{
	static bool is_first = true;
	if (is_first == false)
	{
		return;
	}

	is_first = false;

	queue_initialize(&audio_decode_queue_free);
	queue_initialize(&audio_decode_queue_head);
	ak_thread_mutex_init(&audio_decode_queue_free_mutex, NULL);
	ak_thread_mutex_init(&audio_decode_queue_head_mutex, NULL);
	ak_thread_mutex_init(&audio_output_open_mutex, NULL);
	for (int i = 0; i < AUDIO_DECODE_QUEUE_MAX; i++)
	{
		queue_insert((queue_s *)&audio_decode_buffer[i], &audio_decode_queue_free);
	}
}

bool audio_decode_open(int vol)
{

	if (audio_decode_task_run == true)
	{
		return false;
	}

	if (audio_decode_wait_thread_quit() == false)
	{
		return false;
	}

	audio_decode_queue_init();
	
	audio_decode_task_run = true;
	audio_decode_ready = false;
	ak_pthread_t thread_id;

	audio_decode_device_open(vol,false);
	ak_thread_create(&thread_id, audio_decode_task, (void *)vol, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}

bool audio_decode_queue_push(unsigned char *data, int len)
{
	ak_thread_mutex_lock(&audio_decode_queue_head_mutex);
	if (audio_decode_ready == false)
	{
		ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);
		return false;
	}

	audio_decode_queue *node = audio_decode_queue_node_new(data, len);
	if (node != NULL)
	{
		queue_insert((queue_s *)node, &audio_decode_queue_head);
	}
	ak_thread_mutex_unlock(&audio_decode_queue_head_mutex);

	return true;
}

bool audio_decode_start(bool falg)
{
	if (/*(audio_decode_thread_run == false)||*/ (audio_decode_ready == true))
	{
		return false;
	}
	audio_decode_ready = true;
	return true;
}

bool audio_decode_stop(void)
{
	if (audio_decode_ready == false)
	{
		return false;
	}
	audio_decode_ready = true;
	return true;
}
bool audio_decode_close(void)
{
	if (audio_decode_task_run == false)
	{
		return false;
	}
	audio_decode_task_run = false;
	return true;
}
