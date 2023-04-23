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

     FILE *fp = fopen(argv[3], "rb");
     if (fp == NULL)
     {
          perror("fopen() failed");
          exit(EXIT_FAILURE);
     }

     

}