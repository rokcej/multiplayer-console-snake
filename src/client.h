#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const char DEFAULT_IP[INET_ADDRSTRLEN] = "127.0.0.1";
static const int DEFAULT_PORT = 3000;

int connect_to_server(char *ip, int port);

#endif
