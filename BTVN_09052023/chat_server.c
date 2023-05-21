#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/select.h>
#include <poll.h>


#define MAX_CLIENT 10
#define MAX_LEN 1024

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
    char id[10];
    char name[50];
} client_t;
int main(int argc, char *argv[]){
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

    //socket
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

    client_t clients[MAX_CLIENT];
    int count = 0;

    while(1){
        struct pollfd fds[MAX_CLIENT + 1];
        memset(fds, 0, sizeof(fds));
        fds[0].fd = sockfd;
        fds[0].events = POLLIN;
        int nfds = 1;

        if(count == 0){
            printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                fds[nfds].fd = clients[i].sockfd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }
        int ret = poll(fds,nfds,-1);

        if (ret < 0)
        {
            perror("select() failed");
            continue;
        }

        if (ret == 0)
        {
            printf("poll() timed out\n");
            continue;
        }

        //kt server co san sang hay khong
        if (fds[0].revents & POLLIN)
        {
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t client_len = sizeof(client_addr);
            int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (client < 0)
            {
                perror("accept() failed");
                continue;
            }

            //Them client vao mang
            if(count < MAX_CLIENT)
            {
                clients[count].sockfd = client;
                clients[count].addr = client_addr;
                strcpy(clients[count].id, "");
                strcpy(clients[count].name, "");
                count++;
                printf("Client tu IP %s - PORT: %d ket noi\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // Yeu cau nhap ID va pass
                char *quesToClient = "Vui long nhap \"client_id: client_name\": ";
                if (send(client, quesToClient, strlen(quesToClient), 0) < 0)
                {
                    perror("send() failed");
                    continue;
                }
            }
            else
            {
                printf("Vuot qua so luong client cho phep!");
                printf("\nClinet so %d da ngat ket noi!", client);
                close(client);
            }
        }

        for(int i = 0;i< count; i++)
        {
            if (fds[i + 1].revents & (POLLIN | POLLERR))
            {
                // Nhận dữ liệu từ client
                char str[MAX_LEN];
                memset(str, 0, MAX_LEN);
                int mess_len = recv(clients[i].sockfd, str, MAX_LEN, 0);
                if (mess_len < 0)
                {
                    perror("recv() failed");
                    continue;
                }
                else if(mess_len==0)
                {
                    printf("Client co IP: %s - PORT: %d ngat ket noi\n", inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));

                    //Gui thong bao cho cac client khac
                    char notice[MAX_LEN];
                    sprintf(notice, "Client: %d da ngat ket noi!\n", clients[i].id);
                    for(int j=0;j<count;j++)
                    {
                        
                        
                        if(j!=i && strcmp(clients[j].id,"")!=0)
                        {
                            if(send(clients[j].sockfd,notice, strlen(notice), 0) < 0){
                                perror("send() failed");
                                continue;
                            }
                        }
                    }

                    close(clients[i].sockfd);
                    clients[i] = clients[count-1]; //Xoa client khoi mang
                    count--;

                    // Xoa socket client khoi mang
                    fds[i + 1] = fds[nfds - 1];
                    nfds--;
                    i--;
                    continue;
                }

                else
                {
                    str[strcspn(str, "\n")] = 0;
                    //Kiem tra xem nhap dung cu phat id:name
                    if(strcmp(clients[i].id,"")==0 && strcmp(clients[i].name, "")==0)
                    {
                        char id[10], name[50];
                        if (sscanf(str, "%[^:]: %s", id, name) == 2)
                        {
                            int retCheck = 1;
                            for(int j=0; j<count ;j++)
                            {
                                if(strcmp(clients[j].id, id)==0){
                                    retCheck=0;
                                    break;
                                }
                            }
                            if(retCheck==1)
                            {
                                strcpy(clients[i].id, id);
                                strcpy(clients[i].name, name);
                                printf("Client tu %s:%d ket noi voi ID:NAME: %s:%s\n", inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port), clients[i].id, clients[i].name);
                                char *str = "Ket noi voi ID NAME thanh cong!\n";
                                // if(send(clients[i].sockfd, str, strlen(str), 0 <0)){
                                //     perror("send(1) failed");
                                //     continue;
                                // }
                            }
                            else
                            {
                                char *str = "Client ID da ton tai!\n";
                                 if (send(clients[i].sockfd, str, strlen(str), 0) < 0)
                                {
                                    perror("send() failed");
                                    continue;
                                }
                                str = "Moi nhap lai ID va Name \"client_id: client_name\": ";
                                if (send(clients[i].sockfd, str, strlen(str), 0) < 0)
                                {
                                    perror("send() failed");
                                    continue;
                                }
                            }
                        }
                        else
                        {
                            char *str = "Sai cu phap ID va Name!\n";
                            if (send(clients[i].sockfd, str, strlen(str), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                            str = "Moi nhap lai: \"client_id: client_name\": ";
                            if (send(clients[i].sockfd, str, strlen(str), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                        }
                    }
                    else
                    {
                        char message[MAX_LEN];
                        char recv[20];
                        int ret = sscanf(str, "%[^@]@%s", message, recv);

                        time_t timeNow = time(NULL);
                        struct tm *time = localtime(&timeNow);
                        char timeStr[22];
                        memset(timeStr, 0, 22);
                        strftime(timeStr, MAX_LEN, "%Y/%m/%d %I:%M:%S%p", time);
                        char messSend[MAX_LEN + 44];
                        sprintf(messSend, "%s %s: %s\n", messSend, clients[i].id, str);

                        if (ret==2)
                        {
                            // Gui tin cho nguoi khac
                            int validRev = 0;
                            for (int j = 0;j<count;j++)
                            {
                                if (strcmp(clients[j].id, recv) == 0)
                                {
                                    validRev = 1;
                                    if (send(clients[j].sockfd, messSend, strlen(messSend), 0) < 0)
                                    {
                                        perror("send() failed");
                                        continue;
                                    }
                                    break;
                                }
                            }
                            if (!validRev)
                            {
                                char *str = "Nguoi nhan khong hop le!\n";
                                if (send(clients[i].sockfd, str, strlen(str), 0) < 0)
                                {
                                    perror("send() failed");
                                    continue;
                                }
                            }
                        }
                        //Gui tin den moi nguoi khac
                        else
                        {
                            for (int j = 0; j<count; j++)
                            {
                                if(j != i && strcmp(clients[j].id, "") != 0)
                                {
                                    if(send(clients[j].sockfd, messSend, strlen(messSend), 0) < 0)
                                    {
                                        perror("send() failed");
                                        continue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close (sockfd);
    return 0;
}