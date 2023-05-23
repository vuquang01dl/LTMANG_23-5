#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/select.h>


#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSockets[MAX_CLIENTS], maxClients = MAX_CLIENTS;
    struct sockaddr_in serverAddress, clientAddress;
    int maxDescriptor, activity, i, valread, newSocket;
    int addrlen = sizeof(clientAddress);
    int clientCount = 0;
    char buffer[BUFFER_SIZE];
    char helloMessage[BUFFER_SIZE];

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Tao socket that bai");
        exit(EXIT_FAILURE);
    }

    // Set server socket options
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(9000);

    // Bind server socket to localhost:9000
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections and process data
    while (1) {
        // Clear the socket set
        fd_set readfds;
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(serverSocket, &readfds);
        maxDescriptor = serverSocket;

        // Add client sockets to set
        for (i = 0; i < maxClients; i++) {
            int sd = clientSockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
                if (sd > maxDescriptor)
                    maxDescriptor = sd;
            }
        }

        // Wait for activity on one of the sockets
        activity = select(maxDescriptor + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("Select error");
        }

        // If there is incoming connection request
        if (FD_ISSET(serverSocket, &readfds)) {
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen)) < 0) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            // Send welcome message to the new client
            sprintf(helloMessage, "Xin chào. Hiện có %d clients đang kết nối.\n", clientCount);
            send(newSocket, helloMessage, strlen(helloMessage), 0);

            // Add new client socket to array of sockets
            for (i = 0; i < maxClients; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = newSocket;
                    clientCount++;
                    break;
                }
            }
        }

        // Process data from clients
        for (i = 0; i < maxClients; i++) {
            int sd = clientSockets[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client has disconnected
                    getpeername(sd, (struct sockaddr *)&clientAddress, (socklen_t *)&addrlen);
                    printf("Client disconnected: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    close(sd);
                    clientSockets[i] = 0;
                    clientCount--;
                } else {
                    // Process client message
                    buffer[valread] = '\0';
                    printf("Received message from client %d: %s\n", i, buffer);

                    // Check if client wants to exit
                    if (strcmp(buffer, "exit") == 0) {
                        // Send goodbye message and close connection
                        char goodbyeMessage[] = "Tạm biệt. Kết nối sẽ đóng.\n";
                        send(sd, goodbyeMessage, strlen(goodbyeMessage), 0);
                        close(sd);
                        clientSockets[i] = 0;
                        clientCount--;
                    } else {
                        // Normalize and send message back to client
                        char normalizedMessage[BUFFER_SIZE];
                        int j = 0, k = 0;
                        int len = strlen(buffer);
                        while (j < len) {
                            if (buffer[j] == ' ') {
                                normalizedMessage[k++] = buffer[j++];
                                while (buffer[j] == ' ')
                                    j++;
                            } else if (buffer[j] == '\n' || buffer[j] == '\r') {
                                normalizedMessage[k++] = ' ';
                                j++;
                            } else {
                                if (k == 0 || buffer[j - 1] == ' ')
                                    normalizedMessage[k++] = toupper(buffer[j++]);
                                else
                                    normalizedMessage[k++] = tolower(buffer[j++]);
                            }
                        }
                        normalizedMessage[k] = '\0';
                        send(sd, normalizedMessage, strlen(normalizedMessage), 0);
                    }
                }
            }
        }
    }

    return 0;
}
