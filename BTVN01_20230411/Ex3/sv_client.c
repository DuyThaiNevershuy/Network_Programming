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
     struct sockaddr_in addr;
     {
          addr.sin_family = AF_INET;
          addr.sin_addr.s_addr = inet_addr(argv[1]);
          addr.sin_port = htons(atoi(argv[2]));
     };

     int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
     if (res == -1)
     {
          perror("connect() failed");
          return 1;
     }

     while(1)
     {
          char maSinhVien[256], hoTen[256], ngaySinh[256], diemTB[10], xacNhan[10];

          printf("Moi nhap MSSV: ");
          fgets(maSinhVien, sizeof(maSinhVien), stdin);
          maSinhVien[strcspn(maSinhVien, "\n")] = '\0'; 

          printf("Moi nhap ho ten: ");
          fgets(hoTen, sizeof(hoTen), stdin);
          hoTen[strcspn(hoTen, "\n")] = '\0'; 

          printf("Moi nhap ngay sinh): ");
          fgets(ngaySinh, sizeof(ngaySinh), stdin);
          ngaySinh[strcspn(ngaySinh, "\n")] = '\0'; 

          printf("Moi nhap diem trung binh: ");
          fgets(diemTB, sizeof(diemTB), stdin);
          diemTB[strcspn(diemTB, "\n")] = '\0'; 

          char buf[4*1024];
          memset(buf, 0, 4*1024);
          sprintf(buf, "%s %s %s %s\n", maSinhVien, hoTen, ngaySinh, diemTB);
          send(client, buf, strlen(buf), 0);
     

          printf("Ban co muon nhap tiep? (Y/N): ");
          fgets(xacNhan, sizeof(xacNhan), stdin);
          xacNhan[strcspn(xacNhan,"\n")]='\0';
          fflush(stdin);
          if (xacNhan[0] == 'n' || xacNhan[0] == 'N') {
            break;
          }
     }
     close(client);
}