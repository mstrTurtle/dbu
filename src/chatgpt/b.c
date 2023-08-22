#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_desc, data_sock;
    struct sockaddr_in server;
    const char *message, server_reply[BUFFER_SIZE];
    FILE *file;

    // 创建socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    // 设置FTP服务器的IP和端口
    server.sin_addr.s_addr = inet_addr("FTP_SERVER_IP");
    server.sin_family = AF_INET;
    server.sin_port = htons(21);  // FTP服务器默认端口是21

    // 连接到FTP服务器
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("connect error");
        return 1;
    }

    // 发送USER命令
    message = "USER your_username\r\n";
    if (send(socket_desc, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    // 接收FTP服务器的响应
    if (recv(socket_desc, server_reply, BUFFER_SIZE, 0) < 0) {
        puts("recv failed");
        return 1;
    }
    puts(server_reply);

    // 发送PASS命令
    message = "PASS your_password\r\n";
    if (send(socket_desc, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    // 接收FTP服务器的响应
    if (recv(socket_desc, server_reply, BUFFER_SIZE, 0) < 0) {
        puts("recv failed");
        return 1;
    }
    puts(server_reply);

    // 发送PASV命令以获取数据连接
    message = "PASV\r\n";
    if (send(socket_desc, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    // 接收FTP服务器的响应，解析出数据连接的端口
    if (recv(socket_desc, server_reply, BUFFER_SIZE, 0) < 0) {
        puts("recv failed");
        return 1;
    }
    puts(server_reply);

    // 这里省略解析PASV响应的代码，你需要解析出数据连接的端口并创建新的socket连接到FTP服务器

    // 发送RETR命令以下载文件
    message = "RETR your_file\r\n";
    if (send(socket_desc, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    // 接收FTP服务器的响应
    if (recv(socket_desc, server_reply, BUFFER_SIZE, 0) < 0) {
        puts("recv failed");
        return 1;
    }
    puts(server_reply);

    // 从数据连接接收文件内容并保存到本地
    file = fopen("local_file", "w");
    if (file == NULL) {
        puts("Could not open local file for writing");
        return 1;
    }

    while (1) {
        int bytes_recv = recv(data_sock, server_reply, BUFFER_SIZE, 0);
        if (bytes_recv < 1) {
            break;
        }
        fwrite(server_reply, bytes_recv, 1, file);
    }

    fclose(file);

    close(data_sock);
    close(socket_desc);

    return 0;
}
