#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char *argv[]){
     int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if (listener == -1)
     {
          perror("socket() failed");
          return 1;
     }
     struct sockaddr_in addr;
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(9000);

     if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
     {
          perror("bind() failed");
          return 1;        
     }

     if (listen(listener, 5))
     {
          perror("listen() failed");
          return 1;
     }

     printf("Da khoi tao server thanh cong!\n");

     struct sockaddr_in client_addr;
     int client_addr_len = sizeof(client_addr);

     int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
     printf("Accepted socket %d from IP: %s:%d\n", client, inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

     //Khai bao ten file 
     char *filename_open = argv[2];
     char bufHello[256];     


     FILE *f = fopen(filename_open, "r");
     fgets(bufHello, 256, f);
     fclose(f);
     send(client, bufHello, strlen(bufHello), 0);

     char *filename_recv = argv[3];
     f = fopen(filename_recv,"w");
     char bufRecv[2048];    
     int ret;
     while(1){
          ret = recv(client, bufRecv, sizeof(bufRecv), 0);
          if (ret <= 0){
               break;
          }
          if (ret < sizeof(bufRecv)){
            bufRecv[ret] = 0;
          }

          if(strncmp(bufRecv, "exit",4) == 0){
               break;
          }

          printf("%d bytes received: %s", ret, bufRecv);
          fprintf(f, "%s", bufRecv);
          fclose(f);
     }
 
     close(client);
     close(listener);

}