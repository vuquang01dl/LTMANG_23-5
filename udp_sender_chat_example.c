// client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s 127.0.0.1 9090 9090\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);

    int client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t server_address_length = sizeof(server_address);

    // Tạo socket cho client
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Tao client that bai");
        return 1;
    }

    // Thiết lập địa chỉ và cổng cho client
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(client_port);

    // Gắn socket vào địa chỉ và cổng của client
    if (bind(client_socket, (const struct sockaddr *)&client_address, sizeof(client_address)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Thiết lập địa chỉ và cổng cho server
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    fd_set read_fds;
    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);

        int max_fd = (STDIN_FILENO > client_socket) ? STDIN_FILENO : client_socket;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            return 1;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Đọc dữ liệu từ bàn phím và gửi đến server
            fgets(buffer, MAX_BUFFER_SIZE, stdin);
            sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_address, server_address_length);
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            // Nhận dữ liệu từ server và hiển thị
            memset(buffer, 0, MAX_BUFFER_SIZE);
            recvfrom(client_socket, buffer, MAX_BUFFER_SIZE, 0, NULL, NULL);
            printf("Server: %s", buffer);
        }
    }

    close(client_socket);

    return 0;
}
