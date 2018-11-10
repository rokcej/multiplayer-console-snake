#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_comm.h"
#include "snake.h"

static const int DEFAULT_PORT = 3000;
static const int DEFAULT_CLIENTS = 1;

static const int DEFAULT_WIDTH = 32;
static const int DEFAULT_HEIGHT = 16;

typedef struct _Object {
	int x;
	int y;
} Object;

typedef struct _ObjectList {
	struct _ObjectList* next;
	struct _ObjectList* prev;
	Object element;
} ObjectList;

typedef struct _Client {
	int sockfd;
	char ip[INET_ADDRSTRLEN];

	int length;
	int alive;
	int dir;
	ObjectList *snake;
} Client;

typedef struct _Game {
	int width, height, running;

	int n_clients;
	Client *clients;

	Object fruit;
} Game;

typedef struct _ThreadData {
	int id;
	Game *game;
} ThreadData;

int start_server(int port);
void connect_clients(int server_sockfd, Client *clients, int n_clients);

void start_game(Client *clients, int n_clients);
void spawn_players(Game *game);
void spawn_fruit(Game *game);
void move_snakes(Game *game);
void check_snake_collisions(Game *game);
void check_fruit_collisions(Game *game);
int collides_with(Object object, ObjectList *list);
void *input_handler(void *void_ptr);

void show_score(Client *clients, int n_clients);

#endif
