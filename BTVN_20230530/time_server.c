#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>

#define MAX_CLIENT 10
#define MAX_LEN 1024

void signalHandler()
{
    int stt;
    int pid = wait(&stt);
    if (pid > 0)
    {
        printf("Luong con %d da bi tat voi trang thai %d!\n", pid, stt);
    }
    return;
}

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
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Gan dia chi server vao socket
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
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

    while (1)
    {
        printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);

        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }

        printf("Client dia chi IP %s cong %d da ket noi\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        if (fork() == 0)
        {
            close(sockfd);
            while (1)
            {
                char *mess = "Moi nhap yeu cau: ";
                if (send(client, mess, strlen(mess), 0) < 0)
                {
                    perror("send() failed");
                    break;
                }
                char buf[MAX_LEN];
                memset(buf, 0, MAX_LEN);
                int len = recv(client, buf, MAX_LEN, 0);
                if (len < 0)
                {
                    perror("Receive failed");
                    break;
                }
                else if (len == 0)
                {
                    printf("Client %s:%d da ngat ket noi!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    break;
                }
                else
                {
                    buf[strcspn(buf, "\n")] = 0;
                    if (strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0)
                    {
                        printf("Client %s:%d da ngat ket noi!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        break;
                    }

                    // Xu ly
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
                            if (send(client, buf, strlen(buf), 0) < 0)
                            {
                                perror("send() failed");
                                break;
                            }
                        }
                        else
                        {
                            char *mess = "Sai cau lenh.Moi nhap lai dung dinh dang!\n";
                            if (send(client, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                break;
                            }
                        }
                    }
                    else
                    {
                        char *messs = "Sai cau lenh!\n";
                        if (send(client, mess, strlen(mess), 0) < 0)
                        {
                            perror("send() failed");
                            break;
                        }
                    }
                }
            }
            close(client);
            exit(EXIT_SUCCESS); // Ket thuc child process
        }
        close(client);
    }
    // Dong socket
    close(sockfd);
    return 0;
}
