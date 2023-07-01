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
          exit(EXIT_FAILURE);
     }

     struct sockaddr_in addr;
     memset(&addr, 0, sizeof(addr));
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(9000);

     //gan dia chi cho socket
     if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))==-1)
     {
          perror("bind() failed");
          return 1;        
     }
     //lang nghe ket noi
     if (listen(listener, 5))
     {
          perror("listen() failed");
          return 1;
     }

     printf("Da khoi tao server thanh cong!\n");

     struct sockaddr_in client_addr;
     memset(&client_addr, 0, sizeof(client_addr));
     socklen_t client_addr_len = sizeof(client_addr);
     int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
     if (client == -1)
     {
          perror("accept() failed");
          exit(EXIT_FAILURE);
     }
     printf("Accepted socket %d from IP: %s:%d\n", client, inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));


     //nhan du lieu tu client
     
     char buf[2048];
     int count = 0;
     int i;
     memset(buf, 0 , 2048);
     while(1){
          int ret = recv(client, buf, 2048, 0);
          buf[ret]= '\0';
          char subStr[11]= "0123456789";
          if (ret <= 0){
               break;
          }
          if (ret < sizeof(buf)){
               buf[ret] = 0;
          }

          int lenstr = strlen(buf);
          int lensubtr = strlen(subStr);
          
          if(lenstr<lensubtr) break;
          //cho i chay tu i den lenstr-lensub, 50 ki tu thi duyet 10 lan xem co chuoi 0123456789 hay ko
          for(i=0;i< lenstr- lensubtr; i++){
               if(strncmp(buf+i,subStr, lensubtr)==0){
                    count++;
               }
          }

          printf("Chuoi %s da xuat hien %d lan!\n", subStr, count);
         
     }
     close(client);
     close(listener);
    
}