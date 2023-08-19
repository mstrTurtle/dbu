#include "Downloader.h"
#include "Option.h"

Downloader::Downloader() {}

int run();

int
Downloader::run(Option& option)
{
    ::run();
}

#include <ace/Init_ACE.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>
#include <string>

// FTP服务器的IP地址和端口号
const char *FTP_SERVER_IP = "127.0.0.1";
const u_short FTP_SERVER_PORT = 21;

// 定义控制连接的处理函数
void handle_control_connection(ACE_SOCK_Stream &control_socket, off_t start_offset = 0, size_t size = 100) {
    char buffer[1024];
    ssize_t recv_count;

    // 接收服务器的欢迎信息
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送用户名和密码
    control_socket.send("USER scutech\r\n", 16);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    control_socket.send("PASS dingjia\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送PASV命令，进入被动模式
    control_socket.send("PASV\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 解析被动模式响应，获取数据连接IP和端口号
    unsigned int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(buffer, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    std::string data_ip = std::to_string(ip1) + "." + std::to_string(ip2) + "." +
                          std::to_string(ip3) + "." + std::to_string(ip4);
    u_short data_port = (port1 << 8) + port2;

    // 建立数据连接
    ACE_SOCK_Stream data_socket;
    ACE_INET_Addr data_addr(data_port, data_ip.c_str());
    ACE_SOCK_Connector connector;
    if (connector.connect(data_socket, data_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
        return;
    }

    // 发送REST命令，设置下载的起始偏移量
    // off_t start_offset = 0; // 设置起始偏移量，单位为字节
    ACE_OS::sprintf(buffer, "REST %lu\r\n", start_offset);
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送RETR命令
    const char *file_name = "file_to_download.txt";
    ACE_OS::sprintf(buffer, "RETR %s\r\n", file_name);
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 下载文件
    FILE *file = ACE_OS::fopen(file_name, "wb");
    if (file) {
        ssize_t total_received = 0;
        while ((recv_count = data_socket.recv(buffer, sizeof(buffer))) > 0) {
            ssize_t remaining = recv_count;
            if (total_received + remaining > size) {
                remaining = size - total_received;
            }
            ACE_OS::fwrite(buffer, 1, remaining, file);
            total_received += remaining;
            if (total_received >= size) {
                break;
            }
        }
        ACE_OS::fclose(file);
    }

    // 关闭数据连接
    data_socket.close();

    // 接收RETR命令响应
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送QUIT命令，关闭控制连接
    control_socket.send("QUIT\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
}

int run() {
    // 初始化ACE
    ACE::init();

    // 建立控制连接
    ACE_SOCK_Stream control_socket;
    ACE_INET_Addr control_addr(FTP_SERVER_PORT, FTP_SERVER_IP);
    ACE_SOCK_Connector connector;
    if (connector.connect(control_socket, control_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
        return 1;
    }

    // 处理控制连接
    handle_control_connection(control_socket, 100, 100);

    // 关闭ACE
    ACE::fini();

    return 0;
}

