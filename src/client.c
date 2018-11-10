#include "client.h"

int main(int argc, char *argv[]) {
	// Parameters
	char ip[INET_ADDRSTRLEN];
	strncpy(ip, DEFAULT_IP, INET_ADDRSTRLEN);
	int port = DEFAULT_PORT;
	
	// Connect to server
	int server_sockfd = connect_to_server(ip, port);

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
