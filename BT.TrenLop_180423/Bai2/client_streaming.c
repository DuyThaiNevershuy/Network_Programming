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

     int port = atoi(argv[2]);
     printf("port: %d\n", port);
     char *ip_addr = argv[1];



     struct sockaddr_in addr;
     {
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr("127.0.0.1");
          addr.sin_port = htons(9000);
     };

     //ket noi den server
     int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
     if (res == -1)
     {
          perror("connect() failed");
          return 1;
     }

     FILE *f = fopen("file_streaming.txt", "r");
     if(f==NULL){
          perror("fopen() failed");
          exit(EXIT_FAILURE);
     }

     //doc file va gui du lieu

     char buf[2048];
     while(!feof(f)){
          fgets(buf,sizeof(buf),f);
          printf("Du lieu gui: \n");
          //gui du lieu den server
          if (send(client, buf, strlen(buf), 0) == -1)
          {
               perror("send() failed");
               exit(EXIT_FAILURE);
          }          

     }
     printf("Da gui du lieu thanh cong\n");
     fclose(f);
     close(client);
     return 0;
     
}