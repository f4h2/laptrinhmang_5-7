
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE], client_name[BUFFER_SIZE];
    fd_set read_fds; // Set chứa các file descriptor để sử dụng cho hàm select()
    int max_fd; // Giá trị lớn nhất của các file descriptor để sử dụng cho hàm select()

    // Tạo socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // Kết nối tới server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Nhập tên của client và gửi cho server
    printf("Enter your name: ");
    fgets(client_name, BUFFER_SIZE, stdin);
    client_name[strcspn(client_name, "\n")] = 0;
    snprintf(buffer, BUFFER_SIZE, "client_id: %s", client_name);
    send(client_socket, buffer, strlen(buffer), 0);

    // Nhận và in ra tin nhắn từ server và các client khác
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(client_socket, &read_fds);
        max_fd = (STDIN_FILENO > client_socket) ? STDIN_FILENO : client_socket;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Nhập tin nhắn và gửi tới server
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            snprintf(buffer, BUFFER_SIZE, "%s: %s", client_name, buffer);
            send(client_socket, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(client_socket, &read_fds)) {
            // Nhận tin nhắn từ server hoặc các client khác và in ra màn hình
            bzero(buffer, BUFFER_SIZE);
            recv(client_socket, buffer, BUFFER_SIZE, 0);
            printf("%s\n", buffer);
        }
    }

    // Đóng kết nối và thoát chương trình
    close(client_socket);
    return 0;
}