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
#include <sys/select.h>


#define MAX_CLIENT 10
#define MAX_MSG_LEN 1024

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
    char id[10];
    char name[50];
} client_t;

int main(int argc,char*argv[] ){
     if (argc != 2)
     {
          printf("Usage: %s <port>\n", argv[0]);
          exit(EXIT_FAILURE);
     }

     int server = socket(AF_INET, SOCK_STREAM, 0);
     if (server < 0)
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
     if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
     {
          perror("bind() failed");
          exit(EXIT_FAILURE);
     }

     if (listen(server, MAX_CLIENT) < 0)
     {
          perror("listen() failed");
          exit(EXIT_FAILURE);
     }

     client_t clients[MAX_CLIENT];
     int count = 0;
     while(1){
          fd_set readfdes;
          FD_ZERO(&readfdes);

          FD_SET (server, &readfdes);

          if (count == 0){
               printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);
          }
          else
          {
               for (int i = 0; i < count; i++)
               {
                    FD_SET(clients[i].sockfd, &readfdes);
               }
          }

          if (select(FD_SETSIZE, &readfdes, NULL, NULL, NULL) < 0)
          {
               perror("select() failed");
               continue;
          }

          if (FD_ISSET(server, &readfdes))
          {
               struct sockaddr_in client_addr;
               socklen_t client_addr_len = sizeof(client_addr);
               int client = accept(server, (struct sockaddr *)&client_addr, &client_addr_len);
               if (client < 0)
               {
                    perror("accept() failed");
                    continue;
               }

               if (count < MAX_CLIENT)
               {
                    clients[count].sockfd = client;
                    clients[count].addr = client_addr;
                    strcpy(clients[count].id, "");
                    strcpy(clients[count].name, "");
                    count++;
                    printf("Client dia chi IP %s cong %d da ket noi\n",
                         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
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
               }
          }

          for (int i = 0; i < count; i++)
          {
               if(FD_ISSET(clients[i].sockfd, &readfdes))
               {
                    char msg[MAX_MSG_LEN];
                    memset(msg, 0, MAX_MSG_LEN);
                    int ret = recv(clients[i].sockfd, msg, MAX_MSG_LEN, 0);
                    if (ret < 0)
                    {
                         perror("recv() failed");
                         continue;
                    }
                    else if(ret==0)
                    {
                         //Xoa client khoi mang
                         printf("Client %s:%d da ngat ket noi\n",inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
                         for (int j=i; j<count-1;j++)
                         {
                         clients[j] = clients[j + 1];
                         }
                         count--;

                         // Xoa clinet khoi mang file descriptor
                         FD_CLR(clients[i].sockfd, &readfdes);

                         continue;
                    }

                    else
                    {
                         msg[ret] = '\0';
                         if (strcmp(clients[i].id, "") == 0 && strcmp(clients[i].name, "") == 0)
                         {
                              char id[10], name[50];
                              int res = sscanf(msg, "%[^:]: %s", id, name);
                              printf("%d\n", res);
                              if (res==2)
                              {
                                   strcpy(clients[i].id, id);
                                   strcpy(clients[i].name, name);
                                   printf("Client tu %s:%d ket noi voi ID:NAME: %s:%s\n", inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port), clients[i].id, clients[i].name);
                                   if (send(clients[i].sockfd, "Ban da ket noi thanh cong!\n", 28, 0) < 0)
                                   {
                                        perror("send() failed");
                                        continue;
                                   }
                                   continue;
                              }
                                   
                              else
                              {    //Sai cu phap, Moi nhap lai
                                   printf("Lan 2: %d\n", res);
                                   if (send(clients[i].sockfd, "Error,LOI!\n", 12, 0) < 0)
                                   {
                                        perror("send() failed");
                                        continue;
                                   }
                                   char *quesToClient = "Vui long nhap \"client_id: client_name\": ";
                                   if (send(clients[i].sockfd, quesToClient, strlen(quesToClient), 0) < 0)
                                   {
                                        perror("send() failed");
                                        continue;
                                   }
                                   continue;
                              }
                         }
                         else
                         {
                              time_t nowTime = time(NULL);
                              struct tm *time = localtime(&nowTime);
                              char time_str[22];
                              memset(time_str, 0, 22);
                              strftime(time_str, MAX_MSG_LEN, "%Y/%m/%d %I:%M:%S%p", time);
                              char messages[MAX_MSG_LEN + 50];
                              memset(messages, 0, MAX_MSG_LEN + 50);
                              sprintf(messages, "%s %s: %s", time_str, clients[i].id, msg);
                              for(int j=0;j<count;j++){
                                   if(j!=i){
                                        if(send(clients[j].sockfd, messages, strlen(messages), 0) < 0){
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
     close(server);
     return 0;
}