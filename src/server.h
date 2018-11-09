#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const int DEFAULT_PORT = 3000;
static const int DEFAULT_CLIENTS = 2;

typedef struct _Client {
    int sockfd;
    char ip[INET_ADDRSTRLEN];
} Client;

int start_server(int port);
void connect_clients(int server_sockfd, Client *clients, int n_clients);

#endif
