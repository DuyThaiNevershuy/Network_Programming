#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX_CLIENT 10
#define MAX_LEN 1024
#define NUMBER_THREADS 8

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
} client_t;

void formatTime(char buf[], const char *format)
{
    time_t timeNow = time(NULL);
    struct tm *timeinfo = localtime(&timeNow);
    if (strcmp(format, "dd/mm/yyyy") == 0)
    {
        strftime(buf, MAX_LEN, "%d/%m/%Y\n", timeinfo);
    }
    else if (strcmp(format, "dd/mm/yy") == 0)
    {
        strftime(buf, MAX_LEN, "%d/%m/%y\n", timeinfo);
    }
    else if (strcmp(format, "mm/dd/yyyy") == 0)
    {
        strftime(buf, MAX_LEN, "%m/%d/%Y\n", timeinfo);
    }
    else if (strcmp(format, "mm/dd/yy") == 0)
    {
        strftime(buf, MAX_LEN, "%m/%d/%y\n", timeinfo);
    }
    else
    {
        strcpy(buf, "Sai dinh dang!\n");
    }
}

client_t clients[MAX_CLIENT];
int count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in server_addr;

void *client_proc(void *);
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
    while (1)
    {
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }

        printf("Client dia chi IP %s cong %d da ket noi\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_mutex_lock(&clients_mutex);
        clients[count].sockfd = client;
        clients[count].addr = client_addr;
        count++;
        pthread_mutex_unlock(&clients_mutex);

        // Tao thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_proc, (void *)&clients[count - 1]) != 0)
        {
            perror("pthread_create() failed");
            exit(EXIT_FAILURE);
        }
        pthread_detach(thread_id);
    }
    close(sockfd);
    return 0;
}

void *client_proc(void *param)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);
    client_t client = *(client_t *)param;
    while (1)
    {
        // Gui yeu cau toi client
        char *mess = "Moi nhap lenh: ";
        if (send(client.sockfd, mess, strlen(mess), 0) < 0)
        {
            perror("send() failed");
            break;
        }

        // Nhan yeu cau tu client
        char buf[MAX_LEN];
        memset(buf, 0, MAX_LEN);
        int len = recv(client.sockfd, buf, MAX_LEN, 0);
        if (len < 0)
        {
            perror("recv() failed");
            break;
        }
        else if (len == 0)
        {
            printf("Client %s:%d da ngat ket noi!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            for (int i = 0; i < count; i++)
            {
                if (client.sockfd == clients[i].sockfd)
                {
                    pthread_mutex_lock(&clients_mutex);
                    clients[i] = clients[count - 1];
                    count--;
                    if (count == 0)
                    {
                        printf("Server dang cho tai IP-PORT: %s - %d\n",
                               inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                    }
                    pthread_mutex_unlock(&clients_mutex);
                    close(client.sockfd);
                    break;
                }
            }
            break;
        }
        else
        {
            // Xoa ki tu xuong dong
            buf[strcspn(buf, "\n")] = 0;

            if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
            {
                char *mess = "Ban da thoat server!\n";
                if (send(client.sockfd, mess, strlen(mess), 0) < 0)
                {
                    perror("send() failed");
                    break;
                }

                printf("Client %s:%d da ngat ket noi!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                for (int i = 0; i < count; i++)
                {
                    if (client.sockfd == clients[i].sockfd)
                    {
                        pthread_mutex_lock(&clients_mutex);
                        clients[i] = clients[count - 1];
                        count--;
                        if (count == 0)
                        {
                            printf("Server dang cho tai IP-PORT: %s - %d\n",
                                   inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                        
                        }
                        pthread_mutex_unlock(&clients_mutex);
                        close(client.sockfd);
                        break;
                    }
                }
                break;
            }
            // Xu ly yeu cau tu client
            char cmd[MAX_LEN];
            char format[MAX_LEN];
            char temp[MAX_LEN];

            int ret = sscanf(buf, "%s %s %s", cmd, format, temp);

            if (ret == 2)
            {
                if (strcmp(cmd, "GET_TIME") == 0)
                {
                    memset(buf, 0, MAX_LEN);
                    formatTime(buf, format);
                    if (send(client.sockfd, buf, strlen(buf), 0) < 0)
                    {
                        perror("send() failed");
                        break;
                    }
                }
                else
                {
                    char *mess = "Sai cau lenh.Moi nhap lai dung dinh dang!\n";
                    if (send(client.sockfd, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        break;
                    }
                }
            }
            else
            {
                char *messs = "Sai cau lenh!\n";
                if (send(client.sockfd, mess, strlen(mess), 0) < 0)
                {
                    perror("send() failed");
                    break;
                }
                continue;
            }
        }
    }
    return NULL;
}
