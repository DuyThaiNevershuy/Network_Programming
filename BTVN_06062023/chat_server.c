#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENT 10
#define MAX_LEN 1024

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
    char id[10];
    char name[50];
} client_t;

client_t clients[MAX_CLIENT];
int count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in server_addr;

void *processThread(void *);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiet lap dia chi socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    // Gan dia chi server vao socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Chuyen server sang trang thai lang nghe
    if (listen(sockfd, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }
    printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(server_addr.sin_addr), argv[1]);
    while(1)
    {
        //Chap nhan ket noi tu server
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }
        if (count == MAX_CLIENT)
        {
            char *msg = "Server da day, Vui long thu lai sau!\n";
            if (send(sockfd, msg, strlen(msg), 0) < 0)
            {
                perror("send() failed");
            }
            close(sockfd);
            continue;
        }

        printf("Client dia chi IP %s cong %d da ket noi\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Thêm client vào danh sách
        pthread_mutex_lock(&clients_mutex);
        clients[count].sockfd = client;
        clients[count].addr = client_addr;
        strcpy(clients[count].name, "");
        strcpy(clients[count].id, "");
        count++;
        pthread_mutex_unlock(&clients_mutex);

        //Tao thread de xu ly client
        pthread_t threadID;
        if (pthread_create(&threadID, NULL, processThread, (void *)&clients[count - 1]) != 0)
        {
            perror("pthread_create() failed");
            exit(EXIT_FAILURE);
        }
        pthread_detach(threadID);
    }
    close(sockfd);
    return 0;
}

void *processThread(void *para)
{
    client_t *client = (client_t *)para;
    char buffer[MAX_LEN];
}