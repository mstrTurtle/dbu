#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_desc;
    struct sockaddr_in server;
    char *message, server_reply[BUFFER_SIZE];

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

    close(socket_desc);
    return 0;
}
