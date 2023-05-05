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
int main(int argc, char const *argv[]){

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

     // char *log_file = argv[2];

     // FILE *f = fopen(log_file,"a");

     char buf[1024];
     memset(buf,0,1024);
     while(1){
          // char *log_file = argv[2];
          // FILE *f = fopen(log_file,"a");
          FILE *log_file = fopen(argv[2], "a");

          int ret = recv(client, buf, 1024, 0);
          if (ret <= 0){
               break;
          }
          
          if (ret < sizeof(buf)){
               buf[ret] = 0;
          }

          if(strncmp(buf, "N", 1) == 0){
               break;
          }
          
          time_t curTime = time(NULL);
          char *newTime = ctime(&curTime);
          newTime[strlen(newTime)-1]='\0';

          int received_bytes = recv(client, buf, 1024, 0);
          if (received_bytes < 0) {
               perror("recv() failed");
               continue;
          }

          buf[received_bytes] = '\0';
          fprintf(stdout, "Received data from client with IP %s:\n%s", inet_ntoa(client_addr.sin_addr), buf);
          fprintf(log_file, "%s %s %s\n", inet_ntoa(client_addr.sin_addr), newTime, buf);

          if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) {
               break;
          }

          fclose(log_file);
     }
     close(client);
     close(listener);
    
}