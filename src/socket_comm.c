#include "socket_comm.h"

void send_int(int sockfd, int host_int) {
	int net_int = htonl(host_int);
	int n_bytes = write(sockfd, &net_int, sizeof(int));
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	}
}

int recv_int(int sockfd) {
	int net_int;
	int n_bytes = read(sockfd, &net_int, sizeof(int));
	if (n_bytes < 0) {
		perror("Error reading from socket");
		exit(1);
	}
	int host_int = ntohl(net_int);
	return host_int;
}

void send_ints(int sockfd, int *host_arr, int arr_len) {
	int net_arr[arr_len];
	for (int i = 0; i < arr_len; ++i)
		net_arr[i] = htonl(host_arr[i]);
	int n_bytes = write(sockfd, net_arr, arr_len * sizeof(int));
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	} else if (n_bytes != arr_len * sizeof(int)) {
		fprintf(stderr, "Didn't receive expected number of bytes\n");
	}
}

void recv_ints(int sockfd, int *host_arr, int arr_len) {
	int net_arr[arr_len];
	int n_bytes = read(sockfd, net_arr, arr_len * sizeof(int));
	if (n_bytes < 0) {
		perror("Error reading from socket");
		exit(1);
	} else if (n_bytes != arr_len * sizeof(int)) {
		fprintf(stderr, "Didn't receive expected number of bytes\n");
	}
	for (int i = 0; i < arr_len; ++i)
		host_arr[i] = ntohl(net_arr[i]);
}

void send_bytes(int sockfd, void *buf, int buf_size) {
	int n_bytes = write(sockfd, buf, buf_size);
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	} else if (n_bytes != buf_size) {
		fprintf(stderr, "Warning, didn't send expected number of bytes\n");
	}
}

void recv_bytes(int sockfd, char *buf, int buf_size) {
	int n_bytes = read(sockfd, buf, buf_size);
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	} else if (n_bytes != buf_size) {
		fprintf(stderr, "Warning, didn't receive expected number of bytes\n");
	}
}
