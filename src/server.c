#include "server.h"

int main(int argc, char *argv[]) {
    // Parameters
    int port = DEFAULT_PORT;
    int n_clients = DEFAULT_CLIENTS;

    // Start server
    int server_sockfd = start_server(port);

    // Connect to clients
    Client clients[n_clients];
    connect_clients(server_sockfd, clients, n_clients);

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

        printf("Client #%d connected from %s\n", i, clients[i].ip);
    }

    printf("\n");
}
