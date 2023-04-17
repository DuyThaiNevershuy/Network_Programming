#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
int main(int argc, char *argv[]){
     int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     printf("So tham so dau vao: %d\n",argc);
     printf("Dia chi IP: %s\n", argv[1]);
     printf("Cong(Port): %s\n", argv[2]);

     int port = atoi(argv[2]);
     printf("port: %d\n", port);
     char *ip_addr = argv[1];



     struct sockaddr_in addr;
     {
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr(argv[1]);
          addr.sin_port = htons(atoi(argv[2]));
     };

     int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
     if (res == -1)
     {
          perror("connect() failed");
          return 1;
     }

     // char *msg = "Hello server\n";
     // send(client, msg, strlen(msg), 0);
     
     char buf[2048];
     while(1){
          int ret = recv(client,buf, sizeof(buf), 0);
          printf("Du lieu gui: \n");
          fgets(buf, 256, stdin);
          if(strncmp(buf,"exit",4)==0) break;
          send(client, buf,strlen(buf), 0);
          if (ret==-1) printf("Loi");
          if (ret!=-1) printf("Gui du lieu thanh cong");

     }
     close(client);
     return 0;
     
}