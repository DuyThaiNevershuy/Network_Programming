#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#define BUF_SIZE 1024
struct sockaddr_in *get_data_sock_addr(int, int);
bool login(int);
void upload_file(int, int, char *);
int main(int argc, char *argv[])
{
    // Kiểm tra đầu vào
    if (argc != 3)
    {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Tạo socket
    int control_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (control_sock < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa cho control socket
    struct sockaddr_in control_addr;
    memset(&control_addr, 0, sizeof(control_addr));
    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = inet_addr(argv[1]);
    control_addr.sin_port = htons(atoi(argv[2]));
    socklen_t control_addr_len = sizeof(control_addr);

    // Kết nối tới server
    if (connect(control_sock, (struct sockaddr *)&control_addr, control_addr_len) < 0)
    {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }

    // Nhận thông báo đầu tiên từ server
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    if (recv(control_sock, buf, BUF_SIZE, 0) < 0)
    {
        perror("recv() failed");
        exit(EXIT_FAILURE);
    }
    int reply_code = atoi(buf);
    if (reply_code != 220)
    {
        printf("Error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    int choice = 0;
    bool is_continue = true;
    while (is_continue)
    {
        
    }
}
