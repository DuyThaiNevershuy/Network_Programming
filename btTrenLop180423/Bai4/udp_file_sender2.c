#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>

#define MAX_LENGTH 60
int main(int argc, char *argv[]){
     int sender = socket(AF_INET, SOCK_DGRAM, 0);
     if (sender < 0)
     {
          perror("socket() failed");
          exit(EXIT_FAILURE);
     }

     struct sockaddr_in addr;
     {
          memset(&addr, 0, sizeof(addr));
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr(argv[1]);
          addr.sin_port = htons(atoi(argv[2]));
     };

     FILE *f = fopen(argv[3], "rb");
     if (f == NULL)
     {
          perror("fopen() failed");
          exit(EXIT_FAILURE);
     }

     char buf[MAX_LENGTH + 31];
     memset(buf, 0, MAX_LENGTH + 31);
     strcat(buf, argv[4]);
     // strcat(buf, "/");
     int pos = strlen(buf);


     while (fgets(buf + pos, MAX_LENGTH +1 , f) != NULL){
        printf("Sending %ld bytes: %s\n", strlen(buf), buf);

        if (sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr)) != strlen(buf)){
            perror("sendto() failed");
            exit(EXIT_FAILURE);
        }

        memset(buf + pos, 0,MAX_LENGTH +31 - pos);
        usleep(1000000);
     }
     printf("Gui file %s to %s:%d thanh cong!\n", argv[3], argv[1], atoi(argv[2]));
     printf("Gui file thanh cong!\n");
     fclose(f);
     close(sender);

     return 0;

}