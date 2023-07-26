#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port username\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    // 发送用户名
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "USER %s\r\n", argv[3]);
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    // 读取响应
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("%s", buffer);

    // 发送密码
    bzero(buffer, BUFFER_SIZE);
    printf("Password: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    char *pos = strchr(buffer, '\n');
    if (pos != NULL) {
        *pos = '\0';
    }

    sprintf(buffer, "PASS %s\r\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    // 读取响应
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("%s", buffer);

    // 获取文件
    sprintf(buffer, "RETR /path/to/file\r\n");
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    // 读取响应
    bzero(buffer, BUFFER_SIZE);
    n = read(sockfd, buffer, BUFFER_SIZE);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("%s", buffer);

    // 保存文件
    FILE *file = fopen("file.txt", "w+");
    bzero(buffer, BUFFER_SIZE);

    while ((n = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, sizeof(char), n, file);
    }

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    fclose(file);
    close(sockfd);

    return 0;
}
