#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_comm.h"
#include "snake_comm.h"

static const char DEFAULT_IP[INET_ADDRSTRLEN] = "127.0.0.1";
static const int DEFAULT_PORT = 3000;

typedef struct _ThreadData {
	int server_sockfd;
	int *running;
} ThreadData;

int get_name(char *name, int max_name_len);

int connect_to_server(char *ip, int port, char *name);

void start_game(int server_sockfd);
void *input_handler(void *void_ptr);

void show_scores(int server_sockfd);

#endif
