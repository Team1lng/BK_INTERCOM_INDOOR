#ifndef TCP_SCOKET_H
#define TCP_SCOKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CONNECT_NUM 10
#define READ_BUFFER_LEN 3080

int tcp_init(const char* ip, int port);
int tcp_accept(int sfd);
int tcp_connect(const char* ip, int port);
int tcp_nonblocking_recv(int conn_sockfd, 
                         void *rx_buf, 
                         int buf_len, 
                         int timeval_sec, 
                         int timeval_usec);
int tcp_blocking_recv(int conn_sockfd, void *rx_buf, uint16_t buf_len);
int tcp_send(int conn_sockfd, uint8_t *tx_buf, uint16_t buf_len);
void tcp_close(int sockfd);

#endif