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
#define MAX_LENGTH 1024
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

     char buf[MAX_LENGTH];
    memset(buf, 0, MAX_LENGTH);
    int bytes_received = 0;
    while ((bytes_received = recvfrom(receiver, buf, MAX_LENGTH, 0, NULL, NULL)) > 0)
    {
          printf("Received %d bytes: %s\n", bytes_received, buf);
          char *pos = strchr(buf, '-');
          char filename[31];
          memset(filename, 0, 31);
          strncpy(filename, buf, pos - buf);
          FILE *f = fopen(filename, "ab");
          if (f == NULL)
          {
               perror("fopen() failed");
               exit(EXIT_FAILURE);
          }
          fwrite(pos + 1, 1, strlen(pos + 1), f);
          fclose(f);
          memset(buf, 0, MAX_LENGTH);
    }
     close(receiver);
     return 0;
}