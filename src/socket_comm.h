#ifndef _SOCKET_COMM_H
#define _SOCKET_COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

void send_int(int sockfd, int hdata);
int recv_int(int sockfd);

#endif
