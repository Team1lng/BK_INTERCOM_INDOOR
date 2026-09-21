#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct _queue_s
{
	struct _queue_s *prev;
	struct _queue_s *next;
} queue_s;

void queue_initialize (queue_s *queue);

void queue_insert(queue_s *entry, queue_s *queue);

void queue_delete(queue_s *entry);

queue_s *queue_delete_next(queue_s *queue);

int queue_empty(queue_s *queue);

queue_s *queue_head(queue_s *queue);

queue_s *queue_tail(queue_s *queue);

queue_s *queue_next(queue_s *queue);

queue_s *queue_prev(queue_s *queue);



#endif

