#include "socket_comm.h"

void send_int(int sockfd, int host_data) {
	int net_data = htonl(host_data);
	int n_bytes = write(sockfd, &net_data, sizeof(int));
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	}
}

int recv_int(int sockfd) {
	int net_data;
	int n_bytes = read(sockfd, &net_data, sizeof(int));
	if (n_bytes < 0) {
		perror("Error reading from socket");
		exit(1);
	}
	int host_data = ntohl(net_data);
	return host_data;
}
