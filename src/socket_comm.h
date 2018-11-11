#ifndef _SOCKET_COMM_H
#define _SOCKET_COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void send_int(int sockfd, int host_int);
int recv_int(int sockfd);

void send_ints(int sockfd, int *host_arr, int arr_len);
void recv_ints(int sockfd, int *host_arr, int arr_len);

void send_bytes(int sockfd, void *buf, int buf_size);
void recv_bytes(int sockfd, void *buf, int buf_size);

#endif
