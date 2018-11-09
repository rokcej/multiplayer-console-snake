#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int serv_sock, serv_port;
    struct sockaddr_in serv_addr;
    char *serv_ip;

    // Parameters
    if (argc < 3) {
        printf("Parameter(s): <Server Address> <Server Port>\n");
        exit(0);
    }
    serv_ip = argv[1];
    serv_port = atoi(argv[2]);

    // Socket address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    int rtn_val = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr);
    if (rtn_val == 0) {
        printf("Invalid server address\n");
        exit(0);
    } else if (rtn_val < 0) {
        perror("Unsupported server address");
        exit(1);
    }

    // Main loop
    while (1) {
        // Create socket
        if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error creating socket");
            exit(1);
        }

        // Connect to server
        if (connect(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
            perror("Error connecting to server");
            exit(1);
        }

        // Send message to server
        char buffer[256];
        printf("Enter your message: ");
        fgets(buffer, 256, stdin);
        int n_bytes = write(serv_sock, buffer, strlen(buffer));
        if (n_bytes < 0) {
            perror("Error writing to server socket");
            exit(1);
        }

        // Read message from server
        n_bytes = read(serv_sock, buffer, 255);
        if (n_bytes < 0) {
            perror("Error reading from server socket");
            exit(1);
        }
        buffer[n_bytes] = '\0';
        printf("Server: %s\n\n", buffer);

        // Close socket
        close(serv_sock);
    }

    return 0;
}
