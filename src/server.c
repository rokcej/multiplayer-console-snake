#include "server.h"

int main(int argc, char *argv[]) {
	// Arguments
	if (argc < 2) {
		fprintf(stderr, "Required argument(s): Number of players\n");
		exit(1);
	}

	// Init RNG
	srand(time(NULL));

	// Start server
	int port = DEFAULT_PORT;
	int server_sockfd = start_server(port);

	// Connect to clients
	int n_clients = atoi(argv[1]);
	Client clients[n_clients];
	connect_clients(server_sockfd, clients, n_clients);

	// Game
	start_game(clients, n_clients);
	send_scores(clients, n_clients);

	// Cleanup
	close(server_sockfd);
	for (int i = 0; i < n_clients; ++i)
		close(clients[i].sockfd);

	return 0;
}

int start_server(int port) {
	int server_sockfd;
	struct sockaddr_in server_addr;

	// Create socket
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error creating socket");
		exit(1);
	}

	// Socket address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket to address and listen
	if (bind(server_sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		perror("Error binding socket");
		exit(1);
	}
	if (listen(server_sockfd, 5) < 0) {
		perror("Error listening on socket");
		exit(1);
	}

	// Print status
	printf("Server listening on port %d\n\n", ntohs(server_addr.sin_port));

	return server_sockfd;
}

void connect_clients(int server_sockfd, Client *clients, int n_clients) {
	printf("Waiting for %d clients...\n", n_clients);

	for (int i = 0; i < n_clients; ++i) {
		int client_sockfd;
		struct sockaddr_in client_addr;

		// Accept connection
		int client_addr_len = sizeof(client_addr);
		if ((client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr_len)) < 0) {
			perror("Error accepting connection");
			exit(1);
		}

		// Update client struct
		clients[i].sockfd = client_sockfd;
		if (inet_ntop(AF_INET, &client_addr.sin_addr, clients[i].ip, INET_ADDRSTRLEN) == NULL) {
			perror("Error converting client address to string");
			exit(1);
		}

		// Get client name
		recv_bytes(clients[i].sockfd, clients[i].name, MAX_NAME_LEN - 1);
		clients[i].name[MAX_NAME_LEN - 1] = '\0';

		printf("Client #%d '%s' connected from %s\n", i, clients[i].name, clients[i].ip);

		// User count
		send_int(clients[i].sockfd, n_clients);
		for (int j = 0; j <= i; ++j)
			send_int(clients[j].sockfd, i+1);
	}
	printf("\n");
}

void start_game(Client *clients, int n_clients) {
	printf("Starting game...\n");
	// Init game
	Game game;
	game.tickrate = DEFAULT_TICKRATE;
	game.width = DEFAULT_WIDTH;
	game.height = DEFAULT_HEIGHT;
	game.running = 1;

	game.n_clients = n_clients;
	game.clients = clients;

	// Spawn players and fruit
	spawn_players(&game);
	spawn_fruit(&game);

	// Send game status
	for (int i = 0; i < game.n_clients; ++i)
		send_int(game.clients[i].sockfd, game.running);

	// Create thread for each client
	pthread_t threads[n_clients];
	ThreadData thread_data[n_clients];
	for (int i = 0; i < n_clients; ++i) {
		thread_data[i].id = i;
		thread_data[i].game = &game;
		if (pthread_create(&threads[i], NULL, input_handler, &thread_data[i])) {
			fprintf(stderr, "Error creating thread\n");
			exit(1);
		}
	}
	
	// Main loop
	while (game.running) {
		// Start tick timer
		struct timeval start_tick, stop_tick;
		gettimeofday(&start_tick, NULL);

		// Simulate
		/// Move snakes
		move_snakes(&game);
		/// Check for snake collisions
		check_snake_collisions(&game);
		/// Check for fruit collisions
		check_fruit_collisions(&game);

		// Draw
		/// Create game board
		char board[game.height][game.width];
		memset(&board[0][0], ' ', game.height * game.width);
		board[game.fruit.y][game.fruit.x] = 'X';
		for (int i = 0; i < game.n_clients; ++i) {
			if (game.clients[i].alive) {
				// Tail
				ObjectList *tail = game.clients[i].snake->next;
				while (tail != NULL) {
					board[tail->element.y][tail->element.x] = 'o';
					tail = tail->next;
				}
				// Head
				ObjectList *head = game.clients[i].snake;
				board[head->element.y][head->element.x] = 'O';
			}
		}
		/// Send game information
		for (int i = 0; i < game.n_clients; ++i) {
			// Player info
			send_int(game.clients[i].sockfd, game.clients[i].score);
			send_int(game.clients[i].sockfd, game.clients[i].alive);
			// Board info
			send_int(game.clients[i].sockfd, game.width);
			send_int(game.clients[i].sockfd, game.height);

			int x = game.clients[i].snake->element.x;
			int y = game.clients[i].snake->element.y;
			char temp = board[y][x];
			if (game.clients[i].alive)
				board[y][x] = '0';
			send_bytes(game.clients[i].sockfd, &board[0][0], game.width * game.height);
			board[y][x] = temp;
		}

		// Wait for tick
		gettimeofday(&stop_tick, NULL);
		while ((stop_tick.tv_sec - start_tick.tv_sec + (stop_tick.tv_usec - start_tick.tv_usec) * 1e-6) < (1.0 / game.tickrate)) {
			sleep(0);
			gettimeofday(&stop_tick, NULL);
		}

		// Check and send game status
		int game_over = 1;
		for (int i = 0; i < game.n_clients; ++i) {
			if (game.clients[i].alive) {
				game_over = 0;
				break;
			}
		}
		if (game_over)
			game.running = 0;
		for (int i = 0; i < game.n_clients; ++i)
			send_int(game.clients[i].sockfd, game.running);
	}

	// Join threads
	for (int i = 0; i < n_clients; ++i) {
		pthread_cancel(threads[i]);
		pthread_join(threads[i], NULL);
	}

	printf("Game finished\n\n");
}
void spawn_players(Game *game) {
	int sn = (int) ceil(sqrt((double) game->n_clients));
	int sw = game->width / sn;
	int sh = game->height / sn;
	for (int i = 0; i < game->n_clients; ++i) {
		Client *client = &(game->clients[i]);
		client->length = 1;
		client->score = 0;
		client->alive = 1;
		client->dir = NONE;
		client->dir_pending = NONE;
		client->snake = malloc(sizeof(ObjectList));
		client->snake->next = NULL;
		client->snake->prev = NULL;
		// Spawn coordinates
		client->snake->element.x = (i % sn + 0.5) * sw;
		client->snake->element.y = (i / sn + 0.5) * sh;
	}
}
void spawn_fruit(Game *game) {
	int total_squares = game->width * game->height;
	int taken_squares = 0;
	for (int i = 0; i < game->n_clients; ++i) {
		if (game->clients[i].alive)
			taken_squares += game->clients[i].length;
	}

	int index = rand() % (total_squares - taken_squares);
	for (int y = 0; y < game->height; ++y) {
		for (int x = 0; x < game->width; ++x) {
			int collision = 0;
			for (int i = 0; i < game->n_clients; ++i) {
				Object square = { .x = x, .y = y };
				if (collides_with(square, game->clients[i].snake)) {
					collision = 1;
					break;
				}
			}
			if (!collision) {
				if (index == 0) {
					// Place fruit
					game->fruit.x = x;
					game->fruit.y = y;
					return;
				} else {
					--index;
				}
			}
		}
	}
	// Unable to place fruit
	game->fruit.x = -1;
	game->fruit.y = -1;
}
void move_snakes(Game *game) {
	for (int i = 0; i < game->n_clients; ++i) {
		// Update direction
		game->clients[i].dir = game->clients[i].dir_pending;

		// Move snake
		if (game->clients[i].alive && game->clients[i].dir != NONE) {
			ObjectList *node = game->clients[i].snake;
			while (node->next != NULL)
				node = node->next;
			while (node->prev != NULL) {
				node->element.x = node->prev->element.x;
				node->element.y = node->prev->element.y;
				node = node->prev;
			}
			switch (game->clients[i].dir) {
				case UP:
				node->element.y -= 1;
				if (node->element.y < 0)
					node->element.y = game->height - 1;
				break;

				case RIGHT:
				node->element.x += 1;
				if (node->element.x > game->width - 1)
					node->element.x = 0;
				break;

				case DOWN:
				node->element.y += 1;
				if (node->element.y > game->height - 1)
					node->element.y = 0;
				break;

				case LEFT:
				node->element.x -= 1;
				if (node->element.x < 0)
					node->element.x = game->width - 1;
				break;
			}
		}
	}
}
void check_snake_collisions(Game *game) {
	// Detect collisions
	int kill_client[game->n_clients];
	for (int i = 0; i < game->n_clients; ++i) {
		kill_client[i] = 0;
		if (game->clients[i].alive) {
			for (int j = 0; j < game->n_clients; ++j) {
				if (game->clients[j].alive) {
					Object head = game->clients[i].snake->element;
					ObjectList *snake = game->clients[j].snake;
					if (i == j)
						snake = snake->next;

					if (collides_with(head, snake)) {
						kill_client[i] = 1;
						break;
					}
				}
			}
		}
	}
	// Kill dead snakes
	for (int i = 0; i < game->n_clients; ++i) {
		if (kill_client[i])
			game->clients[i].alive = 0;
	}
}
void check_fruit_collisions(Game *game) {
	for (int i = 0; i < game->n_clients; ++i) {
		if (game->clients[i].alive) {
			Object head = game->clients[i].snake->element;
			Object fruit = game->fruit;
			if (head.x == fruit.x && head.y == fruit.y) {
				ObjectList *node = game->clients[i].snake;
				while (node->next != NULL)
					node = node->next;
				
				node->next = malloc(sizeof(ObjectList));
				node->next->next = NULL;
				node->next->prev = node;
				node->next->element.x = node->element.x;
				node->next->element.y = node->element.y;

				game->clients[i].length += 1;
				game->clients[i].score += 1;

				spawn_fruit(game);
				return;
			}
		}
	}
}
int collides_with(Object object, ObjectList *list) {
	ObjectList *node = list;
	while (node != NULL) {
		if (object.x == node->element.x && object.y == node->element.y)
			return 1;
		node = node->next;
	}
	return 0;
}
void *input_handler(void *void_ptr) {
	ThreadData *data = (ThreadData*) void_ptr;
	int id = data->id;
	Game *game = data->game;
	while (game->running) {
		int new_dir = recv_int(game->clients[id].sockfd);
		switch (new_dir) {
			case UP:
			case DOWN:
			if (game->clients[id].dir == NONE ||
				game->clients[id].dir == LEFT ||
				game->clients[id].dir == RIGHT)
				game->clients[id].dir_pending = new_dir;
			break;

			case LEFT:
			case RIGHT:
			if (game->clients[id].dir == NONE ||
				game->clients[id].dir == UP ||
				game->clients[id].dir == DOWN)
				game->clients[id].dir_pending = new_dir;
			break;

			default:
			break;
		}
	} 
	return NULL;
}

void send_scores(Client *clients, int n_clients) {
	char names[n_clients][MAX_NAME_LEN];
	int scores[n_clients];

	for (int i = 0; i < n_clients; ++i) {
		strncpy(names[i], clients[i].name, MAX_NAME_LEN);
		scores[i] = clients[i].score;
	}

	for (int i = 0; i < n_clients; ++i) {
		send_int(clients[i].sockfd, n_clients);
		send_bytes(clients[i].sockfd, &names[0][0], n_clients * MAX_NAME_LEN);
		send_ints(clients[i].sockfd, scores, n_clients);
	}
}
