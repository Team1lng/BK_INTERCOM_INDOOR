#ifndef _TCP_CLINET_H_
#define _TCP_CLINET_H_
#include"unistd.h"
#include "tcp_socket.h"
#include"ak_thread.h"
#include"ak_common.h"
#if 0
typedef struct 
{
    unsigned int port;
    char *ip;

    unsigned char cmd;
    unsigned char arg1;
    unsigned char arg2;

    char *data;
    unsigned int data_size;

	tcp_device dev_id;
	ak_pthread_t tid ;
	int fd;
}tcp_clinet_t;
#endif
#endif