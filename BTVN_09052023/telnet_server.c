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
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_CLIENT 10
#define MAX_LEN 1024

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
    char username[10];
    char password[50];
} client_t;

int checkAccount(char *username, char *password)
{
    FILE *file = fopen("Account_Data.txt", "r");
    if (file == NULL)
    {
        return -1; // Loi
    }

    // Doc tung dong trong File
    char line[MAX_LEN];
    while (fgets(line, MAX_LEN, file) != NULL)
    {
        line[strcspn(line, "\n")] = 0;
        // Tach ID va password
        char str_username[MAX_LEN];
        char str_password[MAX_LEN];
        if (strcmp(username, str_username) == 0 && strcmp(password, str_password) == 0)
        {
            fclose(file);
            return 1; // So sanh acc nhap vao voi acc file text, dung tra ve 1, sai ve
        }
    }
    fclose(file);
    return 0;
}

void command(int socket, char *command)
{
    char cmd[MAX_LEN];
    sprintf(cmd, "%s > out.txt", command);
    // Thuc thi
    int status = system(cmd);
    if (status == 0)
    {
        FILE *file = fopen("out.txt", "r"); // Mo file out de doc ket qua
        if (file == NULL)
        {
            return;
        }

        // Doc file va gui ve client
        char line[MAX_LEN];
        while (fgets(line, MAX_LEN, file) != NULL)
        {
            if (send(socket, line, strlen(line), 0) < 0)
            {
                perror("send() failed");
                continue;
            }
        }
        char *mess = "Moi nhap yeu cau: ";
        if (send(socket, mess, strlen(mess), 0) < 0)
        {
            perror("send() failed");
            return;
        }
        fclose(file);
    }
    else
    {
        // Gui thong bao loi ve client
        char *mess = "Cau lenh sai. Moi nhap lai cau lenh khac: ";
        if (send(socket, mess, strlen(mess), 0) < 0)
        {
            perror("send() failed");
            return;
        }
    }
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Gan dia chi server vao socket
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    client_t clients[MAX_CLIENT];
    int count = 0;
    while (1)
    {
        fd_set readfdes;
        FD_ZERO(&readfdes);

        FD_SET(sockfd, &readfdes);

        if (count == 0)
        {
            printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                FD_SET(clients[i].sockfd, &readfdes);
            }
        }

        if (select(FD_SETSIZE, &readfdes, NULL, NULL, NULL) < 0)
        {
            perror("select() failed");
            continue;
        }

        if (FD_ISSET(sockfd, &readfdes))
        {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client < 0)
            {
                perror("accept() failed");
                continue;
            }

            if (count < MAX_CLIENT)
            {
                clients[count].sockfd = client;
                clients[count].addr = client_addr;
                strcpy(clients[count].username, "");
                strcpy(clients[count].password, "");
                count++;
                printf("Client dia chi IP %s cong %d da ket noi\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                char *quesToClient = "Vui long nhap \"Username Password\": ";
                if (send(client, quesToClient, strlen(quesToClient), 0) < 0)
                {
                    perror("send() failed");
                    continue;
                }
            }

            else
            {
                char *mess = "Vuot qua so luong client cho phep!\n";
                send(client, mess, strlen(mess), 0);
                close(client);
            }
        }

        for (int i = 0; i < count; i++)
        {
            if (FD_ISSET(clients[i].sockfd, &readfdes))
            {
                char mess[MAX_LEN];
                memset(mess, 0, MAX_LEN);
                int ret = recv(clients[i].sockfd, mess, MAX_LEN, 0);
                if (ret < 0)
                {
                    perror("recv() failed");
                    continue;
                }
                else if (ret == 0)
                {
                    // Xoa client khoi mang
                    printf("Client %s:%d da ngat ket noi\n", inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
                    close(clients[i].sockfd);
                    clients[i] = clients[count - 1];
                    count--;
                    FD_CLR(clients[i].sockfd, &readfdes);
                    continue;
                }
                else
                {
                    mess[strcspn(mess, "\n")] = 0;
                    if (strcmp(clients[i].username, "") == 0 && strcmp(clients[i].password, "") == 0)
                    {
                        // Kiem tra client da dang nhap hay chua
                        char username[MAX_LEN];
                        char password[MAX_LEN];
                        char temp[MAX_LEN];
                        int retMess = sscanf(mess, "%s %s %s", username, password, temp);
                        if (retMess == 2)
                        {
                            // KT thong tin dang nhap,  = 1 thanh cong
                            int stt = checkAccount(username, password);
                            if (stt == 1)
                            {
                                // Luu thong tin account vao client
                                strcpy(clients[i].username, username);
                                strcpy(clients[i].password, password);

                                // Thong bao client
                                char *mess = "Dang nhap thanh cong!\nMoi ban nhap lenh: ";
                                if (send(clients[i].sockfd, mess, strlen(mess), 0) < 0)
                                {
                                    perror("send() failed");
                                    continue;
                                }
                            }
                            else if (stt == 0)
                            {
                                // Thong tin dang nhap sai
                                char *msg = "Tai khoan hoac mat khau sai!\nVui long nhap lai \"username password\": ";
                                if (send(clients[i].sockfd, msg, strlen(msg), 0) < 0)
                                {
                                    perror("send() failed");
                                    continue;
                                }
                            }
                            else
                            {
                                char *mess = "Loi file du lieu!\nVui long nhap lai \"username password\": ";
                                send(clients[i].sockfd, mess, strlen(mess), 0);
                            }
                        }
                        else // Gui ko dung format
                        {
                            char *mess = "Ban da nhap sai dinh dang!\nVui long nhap dung theo dinh dang \"username password\": ";
                            send(clients[i].sockfd, mess, strlen(mess), 0);
                        }
                    }
                    else 
                    {
                        if (strcmp(mess, "quit") == 0 || strcmp(mess, "exit") == 0)
                        {
                            char *mess = "Ban da thoat khoai server!\n";
                            if (send(clients[i].sockfd, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                            close(clients[i].sockfd);
                            //Xoa client ra khoi mang
                            clients[i] = clients[count -1];
                            count --;
                            FD_CLR(clients[i].sockfd, &readfdes);
                        }
                        else
                        {
                            command(clients[i].sockfd,mess);
                        }
                    }
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
