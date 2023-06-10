#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32

typedef struct client
{
    int sockfd;
    struct sockaddr_in addr;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} client_t;

client_t clients[MAX_CLIENTS];
int count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in server_addr;

int authenticate_user(char *, char *);
int handle_command(int, char *);
void *client_proc(void *);

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Thiết lập thông tin địa chỉ cho socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Gan dia chi socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Chuyen socket sang trang thai lang nghe
    if (listen(server, MAX_CLIENTS) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Server dang cho tai %s:%d...\n",
               inet_ntoa(server_addr.sin_addr),
               ntohs(server_addr.sin_port));


    while (1)
    {
        // Chấp nhận kết nối từ client
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);
        int client = accept(server, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }
        if (count == MAX_CLIENTS)
        {
            char *mess = "Server da day. Vui long thu lai sau!\n";
            if (send(client, mess, strlen(mess), 0) < 0)
            {
                perror("send() failed");
            }
            close(client);
            continue;
        }
        printf("Client tu %s:%d da ket noi!\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        pthread_mutex_lock(&clients_mutex);
        clients[count].sockfd = client;
        clients[count].addr = client_addr;
        strcpy(clients[count].username, "");
        strcpy(clients[count].password, "");
        count++;
        pthread_mutex_unlock(&clients_mutex);

        // Tạo thread xu ly client
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_proc, (void *)&clients[count - 1]);
        pthread_detach(thread_id);
    }

    close(server);

    return 0;
}

int authenticate_user(char *username, char *password)
{

    FILE *file = fopen("Account_Data.txt", "r");
    if (file == NULL)
    {
        perror("fopen() failed");
        return -1; // Looix
    }

    char buf[MAX_USERNAME_LEN + MAX_PASSWORD_LEN + 2];
    memset(buf, 0, sizeof(buf));

    // Doc tung dong trong file
    while (fgets(buf, sizeof(buf), file) != NULL)
    {
        // Xoa ky tu xuong dong
        buf[strcspn(buf, "\n")] = 0;

        // Tach user va password
        char stored_username[MAX_USERNAME_LEN];
        char stored_password[MAX_PASSWORD_LEN];
        sscanf(buf, "%s %s", stored_username, stored_password);

        if (strcmp(username, stored_username) == 0 && strcmp(password, stored_password) == 0)
        {
            fclose(file);

            return 1; // Dung user va password thi tra ve 1
        }
    }
    fclose(file);

    return 0; 
}

int handle_command(int sockfd, char *command)
{
    char cmd[BUFFER_SIZE];
    sprintf(cmd, "%s > out.txt", command);

    // Thực thi lệnh
    int status = system(cmd);
    if (status == 0)
    {

        FILE *file = fopen("out.txt", "r");
        if (file == NULL)
        {
            perror("fopen() failed");
            return -1;
        }

        char buf[BUFFER_SIZE];
        memset(buf, 0, sizeof(buf));

        // Doc tung dong trong file
        while (fgets(buf, sizeof(buf), file) != NULL)
        {
            // Gửi dữ liệu về client
            if (send(sockfd, buf, strlen(buf), 0) == -1)
            {
                perror("send() failed");
                return -1;
            }
        }
        fclose(file);
        remove("out.txt");
    }
    return status;
}

void *client_proc(void *param)
{
    client_t *client_info = (client_t *)param;
    char buffer[BUFFER_SIZE];

    // Xác thực client
    while (1)
    {
        char *mess = "Moi nhap \"username password\": ";
        if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
        {
            perror("send() failed");
        }

        // Nhan username va password tu client
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_info->sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0)
        {
            perror("recv() failed");
            return NULL;
        }
        else if (bytes_received == 0)
        {
            printf("Client tu %s:%d da ngat ket noi\n",
                   inet_ntoa(client_info->addr.sin_addr),
                   ntohs(client_info->addr.sin_port));
            for (int i = 0; i < count; i++)
            {
                if (client_info->sockfd == clients[i].sockfd)
                {
                    pthread_mutex_lock(&clients_mutex);
                    clients[i] = clients[count - 1];
                    count--;
                    if (count == 0)
                    {
                        printf("Server dang cho tai %s:%d...\n",
                               inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                    }
                    pthread_mutex_unlock(&clients_mutex);
                    break;
                }
            }
            return NULL;
        }
        else
        {
            buffer[strcspn(buffer, "\n")] = 0;

            char username[MAX_USERNAME_LEN];
            char password[MAX_PASSWORD_LEN];
            char temp[BUFFER_SIZE];
            int ret = sscanf(buffer, "%s %s %s", username, password, temp);
            if (ret == 2)
            {
                // Xac thuc thong tin dang nhap
                int status = authenticate_user(username, password);
                if (status == 1)
                {
                    // Luu thong tin dang nhap vao client
                    strcpy(client_info->username, username);
                    strcpy(client_info->password, password);

                    char *mess = "Dang nhap thanh cong!\n";
                    if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        continue;
                    }
                    break;
                }
                else if (status == 0)
                {
                    // Gửi thông báo đến client
                    char *mess = "Tai khoan hoac mat khau sai. Vui long nhap lai!\n";
                    if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        continue;
                    }
                    continue;
                }
                else
                {
                    char *mess = "LOI! Khong tim thay file du lieu. Vui long thu lai sau!\n";
                    if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        continue;
                    }
                    continue;
                }
            }
            else
            {
                // Gửi thông báo đến client
                char *mess = "Nhap sai dinh dang. Vui long nhap lai!\n";
                if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                {
                    perror("send() failed");
                    continue;
                }
                continue;
            }
        }
    }

    // Xử lý lệnh
    while (1)
    {
        // Yêu cầu client nhập lệnh
        char *mess = "Nhap lenh: ";
        if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
        {
            perror("send() failed");
            continue;
        }

        // Nhận lệnh từ client
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_info->sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received < 0)
        {
            perror("recv() failed");
            return NULL;
        }
        else if (bytes_received == 0)
        {
            printf("Client tu %s:%d da ngat ket noi\n",
                   inet_ntoa(client_info->addr.sin_addr),
                   ntohs(client_info->addr.sin_port));
            for (int i = 0; i < count; i++)
            {
                if (client_info->sockfd == clients[i].sockfd)
                {
                    pthread_mutex_lock(&clients_mutex);
                    clients[i] = clients[count - 1];
                    count--;
                    if (count == 0)
                    {
                        printf("Server dang cho tai %s:%d...\n",
                               inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                    }
                    pthread_mutex_unlock(&clients_mutex);
                    break;
                }
            }
            break;
        }
        else
        {
            // Xóa ký tự xuống dòng ở cuối chuỗi
            buffer[strcspn(buffer, "\n")] = 0;

            if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "exit") == 0)
            {
                // Gửi thông báo đến client
                char *mess = "Ban da thoat khoai server.\n";
                if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                {
                    perror("send() failed");
                    continue;
                }
                printf("Client tu %s:%d dat ngat ket noi!\n",
                       inet_ntoa(client_info->addr.sin_addr),
                       ntohs(client_info->addr.sin_port));
                for (int i = 0; i < count; i++)
                {
                    if (client_info->sockfd == clients[i].sockfd)
                    {
                        pthread_mutex_lock(&clients_mutex);
                        clients[i] = clients[count - 1];
                        count--;
                        if (count == 0)
                        {
                            printf("Server dang cho tai %s:%d...\n",
                                   inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
                        }
                        pthread_mutex_unlock(&clients_mutex);
                        break;
                    }
                }
                break;
            }
            else
            {
                // Xử lý lệnh
                if (handle_command(client_info->sockfd, buffer) != 0)
                {
                    char *mess = "Cau lenh sai. Vui long nhap lai!\n";
                    if (send(client_info->sockfd, mess, strlen(mess), 0) < 0)
                    {
                        perror("send() failed");
                        continue;
                    }
                    continue;
                }
            }
        }
    }

    return NULL;
}