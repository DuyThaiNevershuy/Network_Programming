#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
int main(){
     int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     // printf("So tham so dau vao: %d\n",argc);
     // printf("Dia chi IP: %s\n", argv[1]);
     // printf("Cong(Port): %s\n", argv[2]);

     // int port = atoi(argv[2]);
     // printf("port: %d\n", port);
     // char *ip_addr = argv[1];



     struct sockaddr_in addr;
     {
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr("127.0.0.1");
          addr.sin_port = htons(9000);
     };

     int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
     if (res == -1)
     {
          perror("connect() failed");
          return 1;
     }

     // char *msg = "Hello server\n";
     // send(client, msg, strlen(msg), 0);
     char tenMayTinh[2048];
     char tenODia[2048][2048];
     int soLuongoDia;
     int kichThuocO[2048];
     
     char buf[2048];
     while(1){
          printf("Nhap du lieu may tinh: \n********");

          printf("Ten may tinh: ");
          fgets(tenMayTinh, sizeof(tenMayTinh), stdin);

          printf("So Luong o dia: ");
          scanf("%d",&soLuongoDia);

          for (int i =0;i<soLuongoDia;i++){
               printf("--> Thong tin o dia %d: \n",i+1);
               printf("--> Ten o: ");
               scanf("%s", tenODia[i]);
               printf("--> Kich thuoc o(GB): ");
               scanf("%d", &kichThuocO[i]);
          }
          char diskSpace[2048];
          printf("==%s\nSo O Dia: %d", tenMayTinh, soLuongoDia);
          for (int i=0;i<soLuongoDia;i++){
               sprintf(diskSpace,"##Disk %s: %dGB", tenODia[i],kichThuocO[i]);
               
          }
          printf("\n");
          char Thongtin[2048];
          sprintf(Thongtin, "%s-%s", tenMayTinh,diskSpace);
          send(client, Thongtin, strlen(Thongtin),0);



     }
     close(client);
     return 0;
     
}