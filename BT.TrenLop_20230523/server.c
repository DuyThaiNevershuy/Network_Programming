#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    // Tao socket UDP
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    // Thiết lập địa chỉ và cổng của server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Gán địa chỉ và cổng cho socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind() failed");
        return 1;
    }

    printf("Server dang hoat dong! Doi client tu cong: %s\n", server_addr.sin_port);

    // Mảng lưu trữ các client socket
    int client_sockets[MAX_CLIENTS];
    memset(client_sockets, 0, sizeof(client_sockets));

    // Biến đếm số lượng client đang kết nối
    int num_clients = 0;

    while (1) {
        // Tạo tập file mô tả (descriptor set)
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);

        int max_fd = server_socket;

        // Them cac client vao file des
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_socket = client_sockets[i];

            if (client_socket > 0) {
                FD_SET(client_socket, &read_fds);
                if (client_socket > max_fd) {
                    max_fd = client_socket;
                }
            }
        }

        // Sử dụng hàm select để kiểm tra các socket có dữ liệu để đọc
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("Select() failed");
            return 1;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);

            // Chấp nhận kết nối từ client
            int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (client_socket < 0) {
                perror("Accept() failed");
                return 1;
            }

            // Thêm client socket vào mảng
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    ++num_clients;
                    break;
                }
            }

            // Gửi thông báo số lượng client đang kết nối cho client
            char message[MAX_BUFFER_SIZE];
            snprintf(message, MAX_BUFFER_SIZE, "Co %d clients dang ket noi.\n", num_clients);
            send(client_socket, message, strlen(message), 0);

            printf("Client da ket noi: %s\n", inet_ntoa(client_addr.sin_addr));
        }

        // Kiểm tra xem client socket có sẵn dữ liệu để đọc hay không
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            int client_socket = client_sockets[i];

            if (FD_ISSET(client_socket, &read_fds)) {
                char buffer[MAX_BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                // Nhận dữ liệu từ client
                ssize_t num_bytes = recv(client_socket, buffer, MAX_BUFFER_SIZE - 1, 0);

                if (num_bytes <= 0) {
                    // Đóng kết nối và xóa client socket khỏi mảng
                    close(client_socket);
                    client_sockets[i] = 0;
                    --num_clients;
                } else {
                    // Xử lý tin nhắn từ client
                    buffer[num_bytes] = '\0';

                    // Kiểm tra nếu client gửi "exit"
                    if (strcmp(buffer, "exit") == 0) {
                        char goodbye_message[] = "Ket thuc.\n";
                        send(client_socket, goodbye_message, strlen(goodbye_message), 0);

                        // Đóng kết nối và xóa client socket khỏi mảng
                        close(client_socket);
                        client_sockets[i] = 0;
                        --num_clients;
                    } else {
                        // Chuẩn hóa xâu ký tự và gửi lại kết quả cho client
                        char *token;
                        char *delimiters = " \t\r\n";
                        char response[MAX_BUFFER_SIZE];
                        memset(response, 0, sizeof(response));

                        // Xóa ký tự dấu cách ở đầu xâu
                        token = strtok(buffer, delimiters);
                        if (token != NULL) {
                            strcpy(response, token);
                        }

                        // Xóa ký tự dấu cách ở cuối xâu
                        token = strtok(NULL, delimiters);
                        while (token != NULL) {
                            strcat(response, " ");
                            strcat(response, token);
                            token = strtok(NULL, delimiters);
                        }

                        // Chuẩn hóa chữ cái đầu của từ và viết thường các ký tự còn lại
                        int len = strlen(response);
                        for (int j = 0; j < len; ++j) {
                            if (j == 0 || response[j - 1] == ' ') {
                                response[j] = toupper(response[j]);
                            } else {
                                response[j] = tolower(response[j]);
                            }
                        }

                        send(client_socket, response, strlen(response), 0);
                    }
                }
            }
        }
    }
    close(server_socket);

    return 0;
}