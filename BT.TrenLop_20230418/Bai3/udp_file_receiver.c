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
int main(){
     int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if (receiver < 0)
     {
          perror("socket() failed");
          exit(EXIT_FAILURE);
     }

     struct sockaddr_in addr;
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(9000);

     if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr))==-1)
     {
          perror("bind() failed");
          exit(EXIT_FAILURE);    
     }

     char buf[2048];
     while(1){
          int ret = recvfrom(receiver, buf, sizeof(buf),0,0,0);
          if(ret<=0) break;
          
          buf[ret]=0;
          FILE *f = fopen("file_receive.txt", "a");
          fprintf(f, "%s", buf);
          fclose(f);

          printf("Noi dung nhan duoc: %s", buf);
     }
}