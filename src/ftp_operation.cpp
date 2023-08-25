#include "ftp_operation.h"
#include "FtpUtil.h"
#include "Option.h"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>
#include <string>
#include <thread>

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

/**
 * @brief 建立与FTP服务器的控制连接。
 *
 * 此函数使用给定的IP地址和端口号建立与FTP服务器的控制连接。
 *
 * @param ip FTP服务器的IP地址。
 * @param port FTP服务器的端口号。
 * @return
 * 如果成功建立控制连接，则返回控制连接的套接字；如果出现错误，则返回1。
 */
SOCK connect_to_ftp(Str ip, int port)
{
    // 建立控制连接
    ACE_SOCK_Stream control_socket;
    ACE_INET_Addr control_addr(port, ip.c_str());
    ACE_SOCK_Connector connector;
    if (connector.connect(control_socket, control_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
        return 1;
    }
    return control_socket;
}

/**
 * @brief 登录到FTP服务器。
 *
 * 此函数向FTP服务器发送用户名和密码以进行登录。
 *
 * @param control_socket 控制连接的套接字。
 * @param user FTP服务器的用户名。
 * @param pass FTP服务器的密码。
 * @return 如果成功登录，则返回0；如果出现错误，则返回1。
 */
int login_to_ftp(SOCK control_socket, Str user, Str pass)
{
    char buffer[1024];
    char comm[1024];
    ssize_t recv_count;

    // 接收服务器的欢迎信息
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送用户名和密码
    sprintf(comm, "USER %s\r\n", user.c_str());
    control_socket.send(comm, strlen(comm));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    sprintf(comm, "PASS %s\r\n", pass.c_str());
    control_socket.send(comm, strlen(comm));
    sleep(1);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    return 0;
}

/**
 * @brief 进入被动模式并建立数据连接。
 *
 * 此函数向FTP服务器发送PASV命令以进入被动模式，并解析响应以获取数据连接的IP地址和端口号。
 * 然后，它使用获取的IP地址和端口号建立数据连接。
 *
 * @param control_socket 控制连接的套接字。
 * @param dsock 数据连接的套接字。
 * @return 如果成功建立数据连接，则返回0；如果出现错误，则返回1。
 */
int enter_passive_and_get_data_connection(SOCK control_socket, SOCK& dsock)
{
    char buffer[1024];
    ssize_t recv_count;
    std::cout << "to pasv\n";
    // 发送PASV命令，进入被动模式
    control_socket.send("PASV\r\n", 6);
    std::cout << "sent\n";
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << "recved\n";
    std::cout << buffer;

    // 解析被动模式响应，获取数据连接IP和端口号
    unsigned int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(buffer, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)", &ip1, &ip2,
           &ip3, &ip4, &port1, &port2);
    std::string data_ip = std::to_string(ip1) + "." + std::to_string(ip2) +
                          "." + std::to_string(ip3) + "." +
                          std::to_string(ip4);
    u_short data_port = (port1 << 8) + port2;

    ACE_DEBUG((LM_DEBUG, "got %s, %d\n", data_ip.c_str(), data_port));

    // 建立数据连接
    ACE_SOCK_Stream data_socket;
    ACE_INET_Addr data_addr(data_port, data_ip.c_str());
    ACE_SOCK_Connector connector;
    std::cout << "sleeping\n";
    sleep(2);
    std::cout << "awake\n";
    if (connector.connect(data_socket, data_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
        exit(0);
        return 1;
    }

    dsock = data_socket;
    return 0;
}


void enter_passive_and_get_data_connection(
        Str path,
        off_t off,
        size_t size,
        int part_id,
        FILE* file,
        SOCK sock)
{
    std::cout << "calling downOneSeg " << off << ", " << size << "\n";
    SOCK dsock;
    enter_passive_and_get_data_connection(sock, dsock);
    download_one_segment(sock, dsock, path, off, size, part_id, file);
    quit_and_close(sock);
}

/**
 * @brief 从FTP服务器下载文件的一部分。
 *
 * 此函数使用提供的控制套接字和数据套接字从FTP服务器下载文件的一部分。
 *
 * @param control_socket 用于向FTP服务器发送命令的控制套接字。
 * @param data_socket 用于接收文件数据的数据套接字。
 * @param path FTP服务器上文件的路径。
 * @param start_offset 要下载的部分的起始偏移量。
 * @param size 要下载的部分的大小。
 * @param part_id 正在下载的部分的ID。
 *
 * @note 此函数假设控制套接字和数据套接字已经连接到FTP服务器。
 */
void download_one_segment(
        SOCK control_socket,
        SOCK data_socket,
        Str path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file)
{
    char buffer[1024];
    ssize_t recv_count;

    // 发送TYPE I命令，进入BINARY模式
    // off_t start_offset = 0; // 设置起始偏移量，单位为字节
    ACE_OS::sprintf(buffer, "TYPE I\r\n");
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
    std::cout << "sent part i\n";

    // 发送REST命令，设置下载的起始偏移量
    // off_t start_offset = 0; // 设置起始偏移量，单位为字节
    ACE_OS::sprintf(buffer, "REST %lu\r\n", start_offset);
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
    std::cout << "sent rest\n";

    // 发送RETR命令
    const char* file_name = path.c_str();
    ACE_OS::sprintf(buffer, "RETR %s\r\n", file_name);
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    // if(getStatusCode(buffer) != 550){
    //   std::cout << "open error\n";
    //   exit(0);
    // }
    std::cout << buffer;
    std::cout << "sent retr\n";

    // 下载文件
    std::cout << "I'm " << part_id << " , I will get " << size << "\n";
    ssize_t total_received = 0;
    while ((recv_count = data_socket.recv(buffer, sizeof(buffer))) > 0) {
        std::cout << "I'm " << part_id << " , I received " << recv_count
                  << "\n";
        ssize_t remaining = recv_count;
        if (total_received + remaining > size) {
            remaining = size - total_received;
        }
        ACE_OS::fwrite(buffer, 1, remaining, file);
        std::cout << "I'm " << part_id << " , I wrote " << remaining << "\n";
        total_received += remaining;
        if (total_received >= size) {
            break;
        }
    }

    std::cout << "I'm " << part_id << " , I have got " << total_received
              << "\n";

    // 关闭数据连接
    data_socket.close();

    // 接收RETR命令响应
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
}

/**
 * @brief 关闭控制连接并退出FTP服务器。
 *
 * 此函数发送QUIT命令关闭控制连接，并从FTP服务器退出。
 *
 * @param control_socket 控制连接的套接字。
 * @return 返回值为0表示成功关闭控制连接和退出FTP服务器，否则表示出现错误。
 */
int quit_and_close(ACE_SOCK_Stream& control_socket)
{
    char buffer[1024];
    ssize_t recv_count;

    // 发送QUIT命令，关闭控制连接
    control_socket.send("QUIT\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
    control_socket.close();
    return 0;
}

/**
 * @brief 获取FTP服务器上文件的大小。
 *
 * 此函数向FTP服务器发送SIZE命令以获取指定文件的大小。
 *
 * @param sock 控制连接的套接字。
 * @param path 文件的路径。
 * @return 返回文件的大小（以字节为单位），如果获取失败则返回-1。
 */
int get_ftp_file_size(SOCK sock, const std::string& path)
{
    std::cout << "Getting Ftp Size\n";

    ssize_t recv_count;

    char comm[1000];
    sprintf(comm, "SIZE %s\r\n", path.c_str()); // 发送SIZE
    sock.send(comm, strlen(comm));

    char buffer[1000];
    recv_count = sock.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';

    std::cout << "recieved size. buffer is " << buffer << "\n";

    int size;
    sscanf(buffer, "213 %d", &size); // 接收响应

    return size;
}
