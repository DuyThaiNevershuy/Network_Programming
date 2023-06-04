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
#include <signal.h>
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
        sscanf(line, "%s %s", str_username, str_password);

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
            perror("fail");
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
        remove("out.txt");
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
void signalHandler()
{
    int stt;
    int pid = wait(&stt);
    if (pid > 0)
    {
        printf("Luong con %d da bi tat voi trang thai %d!\n", pid, stt);
    }
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiet lap dia chi socket
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

    // Chuyen server sang trang thai lang nghe
    if (listen(sockfd, MAX_CLIENT) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("\nCho ket noi cua client tai dia chi %s:%s\n", inet_ntoa(addr.sin_addr), argv[1]);

        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }

        printf("Client dia chi IP %s cong %d da ket noi\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Tao child process de xu ly clent
        if (fork() == 0)
        {
            close(sockfd);
        }

        client_t clients;
        memset(&clients, 0, sizeof(clients));
        clients.sockfd = client;
        clients.addr = client_addr;
        strcpy(clients.username, "");
        strcpy(clients.password, "");

        char *quesToClient = "Vui long nhap \"Username Password\": ";
        if (send(client, quesToClient, strlen(quesToClient), 0) < 0)
        {
            perror("send() failed");
            continue;
        }

        char buf[MAX_LEN];
        while (1)
        {
            memset(buf, 0, sizeof(buf));

            int bytes_recv = recv(client, buf, sizeof(buf),0);
            if (bytes_recv < 0)
            {
                perror("recveive failed");
                break;
            }

            else if (bytes_recv == 0)
            {
                printf("Client %s:%d da ngat ket noi!\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                break;
            }
            else
            {
                buf[strcspn(buf, "\n")] = 0;

                if (strcmp(clients.username, "") == 0 && strcmp(clients.password, "") == 0)
                {
                    char username[MAX_LEN];
                    char password[MAX_LEN];
                    char temp[MAX_LEN];
                    int ret = sscanf(buf, "%s %s %s", username, password, temp);
                    if (ret == 2)
                    {
                        int stt = checkAco(username, password);
                        if (stt == 1)
                        {
                            strcpy(clients.username, username);
                            strcpy(clients.password, password);

                            //Gui thong bao den client
                            char *mess = "Dang nhap thanh cong!\nMoi ban nhap lenh: ";
                            if (send(client, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                        }

                        else if(stt == 0)
                        {
                            // Thong tin dang nhap sai
                            char *mess = "Tai khoan hoac mat khau sai!\nVui long nhap lai \"username password\": ";
                            if (send(client, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                        }
                        else
                        {
                            char *mess = "Loi file du lieu!\nVui long nhap lai \"username password\": ";
                            if (send(client, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if (strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0)
                        {
                            char *mess = "Ban da thoat khoai server!\n";
                            if (send(client, mess, strlen(mess), 0) < 0)
                            {
                                perror("send() failed");
                                continue;
                            }
                            break;
                        }
                        else
                        {
                            command(client,buf);
                        }
                    }
                }
            }
            close(client);
            exit(EXIT_SUCCESS); //Thoat child process
        }
        close(client);
    }
    close(sockfd);
    return 0;
}