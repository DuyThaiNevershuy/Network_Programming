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

#define MAX_CLIENT 10
#define MAX_LEN 1024

void formatTime(char buf[], const char*formar)
{
    time_t timeNow= time(NULL);
}

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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

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

    if (listen(sockfd, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);
    char buf[MAX_LEN];

    for(int i=0; i< MAX_CLIENT; i++)
    {
        if(fork() == 0)
        {
            while(1)
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
                printf("Client tu IP:PORT %s:%d da vao process %d:%d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port) ,
                client,getpid());

                int len = recv(client, buf, MAX_LEN,0);
                if (len < 0)
                {
                    perror("recv() failed");
                    exit(EXIT_FAILURE);
                }
                else if (len == 0)
                {
                    printf("Client %s:%d da ngat ket noi\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                    close(client);
                    continue;
                }
                else
                {
                    buf[strcspn(buf, "\n")] = 0;
                    printf("Received %d bytes from client %d: %s\n", len, client, buf);

                    //Gui du lieu cho client
                    char *mess = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";

                    if (send(client, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        exit(EXIT_FAILURE);
                    }
                }
                close(client);
            }
        }
    }
    getchar();
    killpg(0, SIGKILL);
    close(sockfd);
    return 0;
}