#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    int serv_sock, serv_port;
    struct sockaddr_in serv_addr;

    // Parameters
    if (argc < 2) {
        printf("Parameter(s): <Server Port>\n");
        exit(0);
    }
    serv_port = atoi(argv[1]);

    // Create socket
    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Socket address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to address
    if (bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket");
        exit(1);
    }

    // Listen on socket for connections
    if (listen(serv_sock, 5) < 0) {
        perror("Error listening on socket");
        exit(1);
    }

    // Main loop
    while (1) {
        int cli_sock, cli_addr_len;
        struct sockaddr_in cli_addr;

        // Wait for a client to connect
        cli_addr_len = sizeof(cli_addr);
        if ((cli_sock = accept(serv_sock, (struct sockaddr*) &cli_addr, &cli_addr_len)) < 0) {
            perror("Error accepting connection");
            exit(1);
        }

        // Fork process
        int pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            exit(1);
        }
        
        if (pid == 0) {
            // Child

            // Close duplicated socket
            close(serv_sock);

            // Read from client socket
            char buffer[256];
            int n_bytes = read(cli_sock, buffer, 255);
            if (n_bytes < 0) {
                perror("Error reading from client socket");
                exit(1);
            }
            buffer[n_bytes] = '\0';
            printf("Client (%d bytes): %s\n", n_bytes, buffer);

            // Write to client socket
            n_bytes = write(cli_sock, "Message received", 16);
            if (n_bytes < 0) {
                perror("Error writing to client socket");
                exit(1);
            }

            // End process
            exit(0);
        } else {
            // Parent

            // Close connection
            close(cli_sock);
        }
    }

    return 0;
}
