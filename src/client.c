#include "client.h"

int main(int argc, char *argv[]) {
	// Connect to server
	char ip[INET_ADDRSTRLEN];
	strncpy(ip, DEFAULT_IP, INET_ADDRSTRLEN);
	int port = DEFAULT_PORT;
	int server_sockfd = connect_to_server(ip, port);

	// Snake
	start_game(server_sockfd);
	show_score(server_sockfd);

	return 0;
}

int connect_to_server(char *ip, int port) {
	int server_sockfd;
	struct sockaddr_in serv_addr;

	// Print status
	printf("Connecting to %s:%d...\n", ip, port);

	// Socket address
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	int rtn_val = inet_pton(AF_INET, ip, &serv_addr.sin_addr.s_addr);
	if (rtn_val == 0) {
		fprintf(stderr, "Invalid server address\n");
		exit(1);
	} else if (rtn_val < 0) {
		perror("Unsupported server address");
		exit(1);
	}

	// Create socket
	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error creating socket");
		exit(1);
	}

	// Connect to server
	if (connect(server_sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Error connecting to server");
		exit(1);
	}
	printf("Connected to server\n\n");

	// Wait for all clients to connect
	int max_connected = recv_int(server_sockfd);
	int n_connected = recv_int(server_sockfd);
	printf("%d/%d clients connected\n", n_connected, max_connected);
	while (n_connected < max_connected) {
		n_connected = recv_int(server_sockfd);
		printf("%d/%d clients connected\n", n_connected, max_connected);
	}
	printf("\n");

	return server_sockfd;
}

void start_game(int server_sockfd) {
	// Get game status
	int running = recv_int(server_sockfd);

	// Create thread for each client
	pthread_t thread;
	ThreadData data = { .server_sockfd = server_sockfd, .running = &running };
	if (pthread_create(&thread, NULL, input_handler, &data)) {
		fprintf(stderr, "Error creating thread\n");
		exit(1);
	}
	
	while (running) {
		// Get game info
		int width = recv_int(server_sockfd);
		int height = recv_int(server_sockfd);
		char board[height][width];
		recv_bytes(server_sockfd, &board[0][0], width * height);

		// Display board
		system("clear");
		for (int i = 0; i < width + 2; ++i)
			printf("#");
		printf("\n");
		for (int y = 0; y < height; ++y) {
			printf("#");
			for (int x = 0; x < width; ++x) {
				printf("%c", board[y][x]);
			}
			printf("#\n");
		}
		for (int i = 0; i < width + 2; ++i)
			printf("#");
		printf("\n");

		// Get game status
		running = recv_int(server_sockfd);
	}

	// Join threads
	pthread_join(thread, NULL);
}
void *input_handler(void *void_ptr) {
	ThreadData *data = (ThreadData*) void_ptr;
	int server_sockfd = data->server_sockfd;
	int *running = data->running;
	while (*running) {
		int key = getchar();
		switch (key) {
			case 'w':
			send_int(server_sockfd, UP);
			break;

			case 'd':
			send_int(server_sockfd, RIGHT);
			break;

			case 's':
			send_int(server_sockfd, DOWN);
			break;

			case 'a':
			send_int(server_sockfd, LEFT);
			break;

			default:
			break;
		}
	} 
	return NULL;
}

void show_score(int server_sockfd) {
	return;
}
