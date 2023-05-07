#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_socket, max_clients, max_sd, client_socket[MAX_CLIENTS], sd, activity, i, valread;
    struct sockaddr_in server_address, client_address;
    char buffer[BUFFER_SIZE];

    // Tạo socket server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    // Gán địa chỉ server vào socket
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ các client
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    // Xác định số lượng client tối đa
    max_clients = MAX_CLIENTS;

    // Khởi tạo các client socket
    for (i = 0; i < max_clients; i++) {
        client_socket[i] = 0;
    }

    // Chờ kết nối từ các client và xử lý dữ liệu
    while (1) {
        // Thiết lập tập các socket đang hoạt động
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Sử dụng hàm select/poll để chờ các socket sẵn sàng đọc dữ liệu
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("Failed to wait for incoming data");
        }

        // Kiểm tra xem có kết nối mới từ client không
        if (FD_ISSET(server_socket, &readfds)) {
            int new_socket;
            int client_address_size = sizeof(client_address);
            if ((new_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t*) &client_address_size)) < 0) {
                perror("Failed to accept new connection");
                exit(EXIT_FAILURE);
            }

            // Hỏi tên của client
            while (1) {
                bzero(buffer, BUFFER_SIZE);
                valread = read(new_socket, buffer, BUFFER_SIZE);
                if (strcmp(buffer, "") != 0) {
                    break;
                }
            }
            
            // Lưu thông tin client vào mảng client_socket
            for (i = 0; i < max_clients; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        // Kiểm tra các socket khác
        for (i = 0; i < max_clients; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                bzero(buffer, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // Đóng kết nối nếu client ngắt kết nối
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    // Gửi dữ liệu đến các client khác
                    char message[BUFFER_SIZE + 20];
                    bzero(message, BUFFER_SIZE + 20);
                    snprintf(message, BUFFER_SIZE + 20, "%s: %s", inet_ntoa(client_address.sin_addr), buffer);
                    for (int j = 0; j < max_clients; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != sd) {
                            send(client_socket[j], message, strlen(message), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}