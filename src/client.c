#include "client.h"

int main(int argc, char *argv[]) {
	// Arguments
	if (argc < 2) {
		fprintf(stderr, "Required argument(s): Server IP address\n");
		exit(1);
	}

	// Enter name
	char name[MAX_NAME_LEN];
	get_name(name, MAX_NAME_LEN);

	// Connect to server
	char ip[INET_ADDRSTRLEN];
	strncpy(ip, argv[1], INET_ADDRSTRLEN);
	ip[INET_ADDRSTRLEN-1] = '\0';
	int port = DEFAULT_PORT;
	int server_sockfd = connect_to_server(ip, port, name);

	// Snake
	start_game(server_sockfd);
	show_scores(server_sockfd);

	// Cleanup
	close(server_sockfd);

	return 0;
}

int get_name(char *name, int max_name_len) {
	int idx = 0;
	while (idx == 0) {
		printf("Enter your name:\n> ");
		char c;
		while ((c = getchar()) != '\n') {
			if (c == ' ' && (idx < 1 || name[idx-1] == ' '))
				continue;
			if (idx < max_name_len - 1)
				name[idx++] = c;
		}
		while (idx > 0 && name[idx-1] == ' ')
			--idx;
		printf("\n");
	}
	name[idx++] = '\0';
	return idx;
}

int connect_to_server(char *ip, int port, char *name) {
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

	// Send name
	send_bytes(server_sockfd, name, MAX_NAME_LEN - 1);

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
		// Get player info
		int score = recv_int(server_sockfd);
		int alive = recv_int(server_sockfd);
		// Get game info
		int width = recv_int(server_sockfd);
		int height = recv_int(server_sockfd);
		char board[height][width];
		recv_bytes(server_sockfd, &board[0][0], width * height);

		// Display board
		/// Clear screen
		system("clear");
		/// Header
		printf("CONTROLS:\n");
		printf("W = up, A = left, S = down, D = right (Press ENTER to confirm command)\n");
		/// Wall
		for (int i = 0; i < 2*width + 3; ++i)
			printf("#");
		printf("\n");
		/// Board
		for (int y = 0; y < height; ++y) {
			printf("#");
			for (int x = 0; x < width; ++x) {
				printf(" %c", board[y][x]);
			}
			printf(" #\n");
		}
		/// Wall
		for (int i = 0; i < 2*width + 3; ++i)
			printf("#");
		printf("\n");
		/// Score
		printf("SCORE: %d", score);
		if (!alive)
			printf(" (You are dead)\n");
		else
			printf("\n");

		// Get game status
		running = recv_int(server_sockfd);
	}

	// Join threads
	pthread_cancel(thread);
	pthread_join(thread, NULL);
}
void *input_handler(void *void_ptr) {
	ThreadData *data = (ThreadData*) void_ptr;
	int server_sockfd = data->server_sockfd;
	int *running = data->running;
	while (*running) {
		int key = getchar();
		switch (key) {
			case 'W':
			case 'w':
			send_int(server_sockfd, UP);
			break;

			case 'D':
			case 'd':
			send_int(server_sockfd, RIGHT);
			break;

			case 'S':
			case 's':
			send_int(server_sockfd, DOWN);
			break;

			case 'A':
			case 'a':
			send_int(server_sockfd, LEFT);
			break;

			default:
			break;
		}
	} 
	return NULL;
}

void show_scores(int server_sockfd) {
	// Get scores
	int n_players = recv_int(server_sockfd);
	char names[n_players][MAX_NAME_LEN];
	int scores[n_players];
	recv_bytes(server_sockfd, &names[0][0], n_players * MAX_NAME_LEN);
	recv_ints(server_sockfd, scores, n_players);

	// Sort scores
	int ord[n_players];
	for (int i = 0; i < n_players; ++i) {
		ord[i] = i;
		int j = i;
		while (j > 0 && scores[ord[j]] > scores[ord[j-1]]) {
			int tmp_ord = ord[j-1];
			ord[j-1] = ord[j];
			ord[j] = tmp_ord;
			--j;
		}
	}

	// Display scores
	system("clear");
	printf("GAME OVER\n\nResults:\n");

	int nth_place = 1;
	for (int i = 0; i < n_players; ++i) {
		if (i > 0 && scores[ord[i]] != scores[ord[i-1]])
			nth_place = i + 1;
		printf("\t#%d: %s (%d points)\n", nth_place, names[ord[i]], scores[ord[i]]);
	}
	printf("\n");
}
