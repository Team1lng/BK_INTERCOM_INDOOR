#include "queue.h"
#include "stdio.h"

#define XM_ASSERT(x) printf("%s : %d queue fail\n",__func__,__LINE__)

void queue_initialize (queue_s *queue)
{
	queue->prev = queue;
	queue->next = queue;
}
//前插 entry插入queue前
void queue_insert (queue_s *entry, queue_s *queue)
{
	entry->prev = queue->prev;
	entry->next = queue;
	queue->prev->next = entry;
	queue->prev = entry;
}

void queue_delete (queue_s *entry)
{
	if(entry->prev == entry || entry->next == entry)
	{
		XM_ASSERT (0);
	}
	if (entry->next != entry)
	{
		entry->prev->next = entry->next;
		entry->next->prev = entry->prev;
		
		entry->prev = entry;
		entry->next = entry;
	}
}

queue_s * queue_head (queue_s *queue)
{
	if(queue->next == queue)
	{
		XM_ASSERT (0);
	}
	return (queue_s *)queue->next;
}

queue_s * queue_tail (queue_s *queue)
{
	if(queue->next == queue)
	{
		XM_ASSERT (0);
	}
	return (queue_s *)queue->prev;
}

queue_s * queue_delete_next (queue_s *queue)
{
	volatile queue_s	*entry;
	if(queue->next == queue || queue->prev == queue)
	{
		XM_ASSERT (0);
	}
	entry = queue->next;
	queue->next = entry->next;
	entry->next->prev = queue;
	
	entry->prev = (queue_s *)entry;
	entry->next = (queue_s *)entry;
	
	return((queue_s *)entry);
}

queue_s * queue_next (queue_s *queue)
{
	return (queue_s *)queue->next;
}

queue_s * queue_prev (queue_s *queue)
{
	return (queue_s *)queue->prev;
}


int queue_empty (queue_s *queue)
{
	if (queue->next == queue) 
	{
		if(queue->prev != queue)
		{
			XM_ASSERT(0);
			return (-1);
		}
		return(1);
	}
	return(0);
}



