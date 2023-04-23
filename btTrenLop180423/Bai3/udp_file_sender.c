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
     int sender = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

     int port = atoi(argv[2]);
     printf("port: %d\n", port);
     char *ip_addr = argv[1];

     struct sockaddr_in addr;
     {
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr(argv[1]);
          addr.sin_port = htons(atoi(argv[2]));
     };

     char buf[1024];
     char file_name[1024];
     char res[2048];

     FILE *f = fopen(argv[3],"*r");
     if(f==NULL){
          perror("fopen() failed");
          exit(EXIT_FAILURE);
     }

     fgets(buf, sizeof(buf), f);
     fclose(f);

     fprintf(stdout, "File content: %s\n", buf);
     sprintf(res, "%s , %s", argv[3], buf);

     int ret = sendto(sender, res, strlen(res), 0,(struct sockaddr *)&addr, sizeof(addr));
     
     printf("Gui du lieu thanh cong");

}