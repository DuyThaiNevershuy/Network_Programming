#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s server_ip server_port client_port\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);

    // Tao socket UDP
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
    {
        perror("Socket() failed");
        exit(1);
    }

    struct sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address.sin_port = htons(client_port);

    if (bind(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
    {
        perror("Socket() failed");
        close(client_socket);
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        close(client_socket);
        exit(1);
    }

    //
    fd_set readfdes;
    FD_ZERO(&readfdes);
    FD_SET(STDIN_FILENO, &readfdes);
    FD_SET(client_socket, &readfdes);

    printf("Da ket noi server. Ban dau nhap tin nhan:\n");

    while (1)
    {
        fd_set tmp_fds = readfdes;

        // Ham Select de check socket
        if (select(client_socket + 1, &tmp_fds, NULL, NULL, NULL) == -1)
        {
            perror("Socket() failed");
            close(client_socket);
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &tmp_fds))
        {
            char message[MAX_BUFFER_SIZE];
            if (fgets(message, sizeof(message), stdin) == NULL)
            {
                perror("Reading from stdin failed");
                close(client_socket);
                exit(1);
            }

            // Gui tin nhan den server
            if (sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
            {
                perror("Sending message failed");
                close(client_socket);
                exit(1);
            }
        }

        // Kiem tra socket xem co du lieu de doc khong
        if (FD_ISSET(client_socket, &tmp_fds))
        {
            char message[MAX_BUFFER_SIZE];
            struct sockaddr_in server_recv_address;
            socklen_t server_recv_address_len = sizeof(server_recv_address);

            // Nhan tin nhan tu server
            ssize_t recv_len = recvfrom(client_socket, message, sizeof(message) - 1, 0, (struct sockaddr *)&server_recv_address, &server_recv_address_len);
            if (recv_len == -1)
            {
                perror("Receiving message failed");
                close(client_socket);
                exit(1);
            }

            message[recv_len] = '\0';
            printf("[Server]: %s", message);
        }
    }

    close(client_socket);

    return 0;
}
