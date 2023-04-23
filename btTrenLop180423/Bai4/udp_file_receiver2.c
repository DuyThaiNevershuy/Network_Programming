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
     int receiver = socket(AF_INET, SOCK_DGRAM,0);
     if (receiver < 0){
        perror("socket() failed");
        exit(EXIT_FAILURE);
     }

     struct sockaddr_in addr;
     memset(&addr, 0, sizeof(addr));
     addr.sin_family = AF_INET;
     addr.sin_addr.s_addr = htonl(INADDR_ANY);
     addr.sin_port = htons(9000);

     if (bind(receiver, (struct sockaddr *)&addr, sizeof(addr))==-1)
     {
          perror("bind() failed");
          exit(EXIT_FAILURE);    
     }
}