#include "socket_comm.h"

void send_int(int sockfd, int host_int) {
	send_ints(sockfd, &host_int, 1);
}

int recv_int(int sockfd) {
	int host_int;
	recv_ints(sockfd, &host_int, 1);
	return host_int;
}

void send_ints(int sockfd, int *host_arr, int arr_len) {
	int net_arr[arr_len];
	for (int i = 0; i < arr_len; ++i)
		net_arr[i] = htonl(host_arr[i]);
	send_bytes(sockfd, net_arr, arr_len * sizeof(int));
}

void recv_ints(int sockfd, int *host_arr, int arr_len) {
	int net_arr[arr_len];
	recv_bytes(sockfd, net_arr, arr_len * sizeof(int));
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

void recv_bytes(int sockfd, void *buf, int buf_size) {
	int n_bytes = read(sockfd, buf, buf_size);
	if (n_bytes < 0) {
		perror("Error writing to socket");
		exit(1);
	} else if (n_bytes != buf_size) {
		fprintf(stderr, "Warning, didn't receive expected number of bytes\n");
	}
}
