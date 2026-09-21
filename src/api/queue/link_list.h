#ifndef _LINK_LIST_H_
#define _LINK_LIST_H_

#include "list.h"
#include "stdint.h"
#include "ak_thread.h"

#define DEFAULT_BLOCK_NUM 10
#define CONSUMER_MAX 8
#define PRINT_ERROR(format, ...) printf("\033[0;31m[%s : %d] " format "\033[0m", __func__, __LINE__, ##__VA_ARGS__)
#define PRINT_NORMAL(format, ...) printf("\033[0;32m[%s : %d] " format "\033[0m", __func__, __LINE__, ##__VA_ARGS__)
#define PRINT_INFO(format, ...) printf("\033[0;33m[%s : %d] " format "\033[0m", __func__, __LINE__, ##__VA_ARGS__)
#define PRINT_WARNING(format, ...) printf("\033[0;35m[%s : %d] " format "\033[0m", __func__, __LINE__, ##__VA_ARGS__)

typedef enum
{
    AI_CAPTURE_LIST,
    VOICE_INFO_LIST,
    VOICE_PALY_LIST,
    NET_AU_LIST,
    NET_VENC_LIST,
    NET_VDEC_LIST,
    TOTAL_LIST
} list_type;

typedef struct
{
    int (*read)(void **dest, void *sour);
    int (*wirte)(void *dest, void *sour);
    int (*release)(void *sour);
    /* The data structure size of void*data in the list_node is stored here */
    uint8_t data_struct_size;
} list_operations;

typedef struct
{
    void *data;
    struct list_head list;
} list_node;

typedef struct
{
    struct list_head head;
    uint8_t list_size;

    int list_pool;

    int node_pool;

    int enable;

    int wait;

    long long consumer[CONSUMER_MAX];

    unsigned long long first_index;

    uint8_t block;

    ak_mutex_t mutex;

    ak_cond_t cond;

    list_operations fops;
} list_attr;

static inline void INIT_LIST_ATTR(list_attr *list)
{
    INIT_LIST_HEAD(&list->head);

    list->list_size = 0;
    list->list_pool = -1;
    list->node_pool = -1;
    list->block = DEFAULT_BLOCK_NUM;
    list->enable = 0;
    list->wait = 0;
    list->first_index = 0;

    ak_thread_mutex_init(&list->mutex, NULL);
    ak_thread_cond_init(&list->cond);

    list->fops.read = NULL;
    list->fops.wirte = NULL;
    list->fops.release = NULL;
}

/*
 *	list_init - init an list queue attr
 *	notes:The list must be initialized for the audio module to work properly
 */
int list_init(void);

/*
 *	list_init - Creates a list queue of the specified type
 *	notes:
 */
int list_create(list_type type, list_operations fops, int wait);

/*
 *	list_node_clear - Clears the list queue of the specified type
 *	notes:
 */
int list_node_clear(list_type type);

/*
 *	list_dsetroy - Dsetroy the list queue of the specified type
 *	notes:
 */
int list_dsetroy(list_type type);

/*
 *	list_node_in - Queue node insertion
 *	notes:
 */
int list_node_in(list_type type, void *sour);

/*
 *	list_node_out - Queue node fetch
 *	notes:
 */
int list_node_out(list_type type, int consumer, void **dest);

/*
 *	list_node_release - Queue node release
 *	notes:
 */
int list_node_release(list_type type, void *data);

/*
 *	list_enable_ctrl - List enable control
 *	notes:
 */
int list_enable_ctrl(list_type type, int status);

/*
 *	list_consumer_register - Register a consumer to retrieve the list data
 *	notes:
 */
int list_consumer_register(list_type type);
#endif