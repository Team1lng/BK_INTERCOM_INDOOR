#include "ak_mem.h"
#include "link_list.h"
#include "list.h"

#include "string.h"

static list_attr list_group[TOTAL_LIST];
static int list_init_flag = -1;
unsigned long long os_get_us(void)
{
	struct ak_timeval tv;
	ak_get_ostime(&tv);
	return tv.usec + tv.sec * 1000 * 1000;
}

/*
 *	list_init - init an list queue attr
 *	notes:The list must be initialized for the audio module to work properly
 */
int list_init(void)
{
	if (list_init_flag == -1)
	{
		for (int i = 0; i < TOTAL_LIST; i++)
		{
			INIT_LIST_ATTR(&(list_group[i]));
			memset(list_group[i].consumer, -1, sizeof(list_group[i].consumer));
		}
		list_init_flag = 0;
		return AK_SUCCESS;
	}
	return AK_FAILED;
}

/*
 *	list_init - Creates a list queue of the specified type
 *	notes:
 */
int list_create(list_type type, list_operations fops, int wait)
{
	if (type >= TOTAL_LIST)
	{
		PRINT_ERROR("[A list type error !!!]\n");
		return AK_FAILED;
	}

	if (list_group[type].list_pool != -1)
	{
		PRINT_ERROR("[A type:[%d] list_pool has been created!]\n", type);
		return AK_FAILED;
	}

	if (ak_mem_pool_init(&(list_group[type].list_pool), MODULE_ID_APP, list_group[type].block, sizeof(list_node)) != AK_SUCCESS)
	{
		PRINT_ERROR("[type[%d] list_pool create fail......]!\n", type);
		goto CREATE_LIST_POOL_FAIL;
	}
	else
	{
		PRINT_INFO("[type[%d] list_pool create %d cache succeed......]!\n", type, list_group[type].block);
	}

	if (ak_mem_pool_init(&(list_group[type].node_pool), MODULE_ID_APP, list_group[type].block, fops.data_struct_size) != AK_SUCCESS)
	{
		PRINT_ERROR("[type[%d] node_pool %d cache create fail......]!\n", type, list_group[type].block);
		goto CREATE_NODE_POOL_FAIL;
	}
	else
	{
		PRINT_INFO("[type[%d] node_pool create %d cache succeed......]!\n", type, list_group[type].block);
	}
	list_group[type].fops = fops;
	list_group[type].enable = 0;
	list_group[type].wait = wait;
	list_group[type].first_index = 0;

	return AK_SUCCESS;

CREATE_NODE_POOL_FAIL:
	ak_mem_pool_destroy(list_group[type].list_pool);

CREATE_LIST_POOL_FAIL:
	return AK_FAILED;
}

/*
 *	list_node_clear - Clears the list queue of the specified type
 *	notes:
 */
int list_node_clear(list_type type)
{
	if (type >= TOTAL_LIST)
	{
		PRINT_ERROR("[A list type error !!!]\n");
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&list_group[type].mutex);
	if (list_group[type].list_pool == -1)
	{
		// PRINT_ERROR("[A list cache list_pool not yet created !!!]\n");
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_FAILED;
	}

	list_node *pos = NULL;

	while (!list_empty(&(list_group[type].head)))
	{
		pos = list_first_entry(&(list_group[type].head), list_node, list);
		list_del(&pos->list);

		if (list_group[type].fops.release != NULL)
			list_group[type].fops.release(pos->data);

		ak_mem_pool_free(pos->data, list_group[type].node_pool);
		ak_mem_pool_free(pos, list_group[type].list_pool);
		list_group[type].list_size--;
	}

	if (list_group[type].list_size != 0)
	{
		PRINT_ERROR("[The list memory pool of this type {%d] has not been released, and there are %d caches remaining......]!\n", type, list_group[type].list_size);
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_FAILED;
	}

	ak_thread_mutex_unlock(&list_group[type].mutex);

	PRINT_INFO("[The type[%d] list_pool clear %d cache succeed......]!\n", type, list_group[type].block);
	return AK_SUCCESS;
}

/*
 *	list_dsetroy - Dsetroy the list queue of the specified type
 *	notes:
 */
int list_dsetroy(list_type type)
{
	ak_thread_mutex_lock(&list_group[type].mutex);
	/* Disable node application */
	list_group[type].enable = -1;
	ak_thread_mutex_unlock(&list_group[type].mutex);

	if (list_node_clear(type) == AK_SUCCESS)
	{
		ak_thread_mutex_lock(&list_group[type].mutex);
		while (list_group[type].list_size)
		{
			PRINT_WARNING("[Wait until the remaining %d cache nodes of the %d type are released......]!\n", list_group[type].list_size, type);
			ak_sleep_ms(1);
		}

		ak_mem_pool_destroy(list_group[type].node_pool);
		ak_mem_pool_destroy(list_group[type].list_pool);
		list_group[type].list_pool = -1;
		list_group[type].node_pool = -1;
		memset(list_group[type].consumer, -1, sizeof(list_group[type].consumer));
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_SUCCESS;
	}
	return AK_FAILED;
}

/*
 *	list_node_in - Queue node insertion
 *	notes:
 */
int list_node_in(list_type type, void *sour)
{
	// unsigned long long x = os_get_us();
	if (type >= TOTAL_LIST)
	{
		PRINT_ERROR("[A list type error !!!]\n");
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&list_group[type].mutex);

	if (list_group[type].list_pool == -1 || list_group[type].enable == -1)
	{
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_FAILED;
	}

	list_node *new_node = NULL;
	if (list_group[type].list_size >= list_group[type].block)
	{
		if (list_empty(&(list_group[type].head)))
		{
			PRINT_ERROR("[The type :[%d] memory of the linked list node needs to be freed !!!]\n", type);
			ak_thread_mutex_unlock(&list_group[type].mutex);
			return AK_FAILED;
		}

		new_node = list_first_entry(&(list_group[type].head), list_node, list);
		/* Delete the oldest node */
		list_del(&new_node->list);
		/* Node data clearing */
		if (list_group[type].fops.release != NULL)
			list_group[type].fops.release(new_node->data);

		list_group[type].first_index++;
	}
	else
	{
		if (ak_mem_pool_alloc((void *)&new_node, list_group[type].list_pool) != AK_SUCCESS || new_node == NULL)
		{
			PRINT_ERROR("Alloc new node failed!\n");
			goto ALLOC_NODE_FAIL;
		}

		if (ak_mem_pool_alloc(&(new_node->data), list_group[type].node_pool) != AK_SUCCESS || new_node->data == NULL)
		{
			PRINT_ERROR("Alloc node data opinter failed!\n");
			goto ALLOC_CACHE_FAIL;
		}

		list_group[type].list_size++;
	}

	if (list_group[type].fops.wirte != NULL)
		list_group[type].fops.wirte(new_node->data, sour);

	list_add_tail(&(new_node->list), &(list_group[type].head));
	ak_thread_mutex_unlock(&list_group[type].mutex);

	// PRINT_INFO("Alloc new node insert succee,curr list num:%d,first_index:%llu,Elapsed Time:%llu us!\n", list_group[type].list_size, list_group[type].first_index, os_get_us());
	if (list_group[type].wait)
		ak_thread_cond_broadcast(&list_group[type].cond);

	return AK_SUCCESS;

ALLOC_CACHE_FAIL:
	ak_mem_pool_free(new_node, list_group[type].list_pool);
ALLOC_NODE_FAIL:
	ak_thread_mutex_unlock(&list_group[type].mutex);
	return AK_FAILED;
}

/*
 *	list_fine_offset -  Finds the specified offset node
 *	notes:
 */
list_node *list_fine_offset(struct list_head *head, int offset)
{
	list_node *pos;
	int index = 0;

	if (list_empty(head))
	{
		return NULL;
	}

	for (pos = list_first_entry(head, list_node, list);; index++)
	{
		if (offset == index)
		{
			return pos;
		}

		if (pos->list.next == head)
		{
			return NULL;
		}

		pos = list_first_entry(&(pos->list), list_node, list);
	}
	return NULL;
}

/*
 *	list_node_out - Queue node fetch
 *	notes:
 */
int list_node_out(list_type type, int consumer, void **dest)
{
	if (type >= TOTAL_LIST || list_group[type].consumer[consumer] == -1)
	{
		PRINT_ERROR("[A list type[%d] or consumer[%lld] error !!!]\n", type, list_group[type].consumer[consumer]);
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&list_group[type].mutex);
	if (list_group[type].list_pool == -1)
	{
		PRINT_ERROR("[A list cache list_pool not yet created !!!]\n");
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_FAILED;
	}

	if (list_group[type].consumer[consumer] < list_group[type].first_index)
	{
		list_group[type].consumer[consumer] = list_group[type].first_index;
	}

	if (list_group[type].wait)
	{
		while (list_group[type].consumer[consumer] == list_group[type].first_index + list_group[type].list_size)
		{
			ak_thread_cond_wait(&list_group[type].cond, &list_group[type].mutex);
			if (list_group[type].enable == -1)
			{
				ak_thread_mutex_unlock(&list_group[type].mutex);
				return AK_FAILED;
			}
		}
	}
	else
	{
		if (list_group[type].consumer[consumer] == list_group[type].first_index + list_group[type].list_size)
		{
			ak_thread_mutex_unlock(&list_group[type].mutex);
			return AK_FAILED;
		}
	}

	list_node *pos = list_fine_offset(&(list_group[type].head), list_group[type].consumer[consumer] - list_group[type].first_index);
	if (pos == NULL)
	{
		PRINT_ERROR("Node not found!!! index:%lld,first_index:%llu\n", list_group[type].consumer[consumer], list_group[type].first_index);
		ak_thread_mutex_unlock(&list_group[type].mutex);
		return AK_FAILED;
	}

	if (list_group[type].fops.read != NULL)
		list_group[type].fops.read(dest, pos->data);

	static long long prev_index = -1;
	if (prev_index + 1 != list_group[type].consumer[consumer])
	{
		PRINT_WARNING("spik framr,consumer:%lld!\n", prev_index + 1);
	}
	prev_index = list_group[type].consumer[consumer];
	list_group[type].consumer[consumer]++;
	// PRINT_INFO("Alloc new node output succee,consumer:%lld!\n", list_group[type].consumer[consumer]);

	ak_thread_mutex_unlock(&list_group[type].mutex);

	/* The node can be freed here because node_info in the node is extra memory allocated,
	 *	so it is not affected by the node freeing*/

	return AK_SUCCESS;
}

/*
 *	list_node_release - Queue node release
 *	notes:
 */
int list_node_release(list_type type, void *data)
{
	if ((data) == NULL)
	{
		PRINT_ERROR("[Failed to release the node because the node is empty !!!]\n");
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&list_group[type].mutex);
	ak_mem_pool_free(data, list_group[type].node_pool);

	list_group[type].list_size--;
	ak_thread_mutex_unlock(&list_group[type].mutex);

	return AK_FAILED;
}

/*
 *	list_enable_ctrl - List enable control
 *	notes:
 */
int list_enable_ctrl(list_type type, int status)
{
	ak_thread_mutex_lock(&list_group[type].mutex);
	if (list_group[type].list_pool != -1)
	{
		list_group[type].enable = status;
		ak_thread_mutex_unlock(&list_group[type].mutex);

		if (list_group[type].wait)
			ak_thread_cond_broadcast(&list_group[type].cond);

		PRINT_INFO("[%s the type:[%d]list function !!!]\n", status == 0 ? "Enable" : "Disable", type);
		return AK_SUCCESS;
	}
	ak_thread_mutex_unlock(&list_group[type].mutex);
	return AK_FAILED;
}

/*
 *	list_consumer_register - Register a consumer to retrieve the list data
 *	notes:
 */
int list_consumer_register(list_type type)
{
	for (int i = 0; i < CONSUMER_MAX; i++)
	{
		if (list_group[type].list_pool != -1 && list_group[type].consumer[i] == -1)
		{
			list_group[type].consumer[i] = 0;
			PRINT_INFO("[type:%d,consumer:%llu,index :%d!!!]\n", type, list_group[type].consumer[i], i);
			return i;
		}
	}
	PRINT_ERROR("[Too many consumers ,type:%d!!!]\n", type);
	return -1;
}