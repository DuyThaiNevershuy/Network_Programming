#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define DEFAULT_ROOT "./"
#define MAX_LEN 1024
#define DEFAULT_PORT 8080

void send_response(int client_socket, const char *message) {
    char response[MAX_LEN];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n%s", strlen(message), message);
    send(client_socket, response, strlen(response), 0);
}

void executeDir(int client_socket, const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        send_response(client_socket, "404 ERROR");
        return;
    }

    struct dirent *dir_entry;
    char response[MAX_LEN];
    sprintf(response, "<html><body><ul>");

    // Read directory contents
    while ((dir_entry = readdir(dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        char entry_path[MAX_LEN];
        sprintf(entry_path, "%s/%s", dir_path, dir_entry->d_name);

        struct stat entry_stat;
        if (stat(entry_path, &entry_stat) == 0) {
            if (S_ISDIR(entry_stat.st_mode)) {
                // Directory entry
                sprintf(response + strlen(response), "<li><b><a href=\"%s/\">%s/</a></b></li>", dir_entry->d_name, dir_entry->d_name);
            } else {
                // File entry
                sprintf(response + strlen(response), "<li><i><a href=\"%s\">%s</a></i></li>", dir_entry->d_name, dir_entry->d_name);
            }
        }
    }

    sprintf(response + strlen(response), "</ul></body></html>");
    send_response(client_socket, response);

    closedir(dir);
}

void executeFile(int client_socket, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        send_response(client_socket, "404 ERROR");
        return;
    }

    // Doc noi dung file
    fseek(file, 0, SEEK_END);
    long sizeFile = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *file_content = malloc(sizeFile + 1);
    fread(file_content, sizeFile, 1, file);
    file_content[sizeFile] = '\0';

    send_response(client_socket, file_content);

    free(file_content);
    fclose(file);
}

void requestCommand(int client_socket, const char *request, const char *root) {
    char method[MAX_LEN], path[MAX_LEN], protocol[MAX_LEN];
    sscanf(request, "%s %s %s", method, path, protocol);

    // Append root directory to the requested path
    char file_path[MAX_LEN];
    sprintf(file_path, "%s%s", root, path);

    if (strcmp(method, "GET") == 0) {
        struct stat path_stat;
        if (stat(file_path, &path_stat) == 0) {
            if (S_ISDIR(path_stat.st_mode)) {
                executeDir(client_socket, file_path);
            } else if (S_ISREG(path_stat.st_mode)) {
                executeFile(client_socket, file_path);
            } else {
                send_response(client_socket, "404 Not Found");
            }
        } else {
            send_response(client_socket, "404 Not Found");
        }
    } else {
        send_response(client_socket, "501 Not Implemented");
    }
}

int main(int argc, char *argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_size = sizeof(client_address);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Thiet lap dia chi socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(DEFAULT_PORT);

    // Gan dia chi server vao socket
    if (bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    // Chuyen server sang trang thai lang nghe
    if (listen(server, 10) < 0)
    {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Server dang cho tai cong: %d\n", DEFAULT_PORT);

    const char *root = DEFAULT_ROOT;
    if (argc >= 2) {
        root = argv[1];
    }

    while(1)
    {
        client_socket = accept(socket, (struct sockaddr *)&client_address, &client_address_size);
        if(client_socket < 0)
        {
            perror("Connect() fail");
            return 1;
        }

        char request[MAX_LEN];
        memset(request, 0, MAX_LEN);

        // Doc yeu cau cua client
        recv(client_socket, request, MAX_LEN - 1, 0);
        printf("Request:\n%s\n", request);

        // Thuc hien yeu cau cua client
        requestCommand(client_socket, request, root);

        // Client ngat ket noi
        close(client_socket); 
    }
    close(server);
    return 0;
}