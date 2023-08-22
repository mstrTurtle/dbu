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

using Str = std::string;


SOCK connectToFtp(Str ip, int port=21){
    // 建立控制连接
    ACE_SOCK_Stream control_socket;
    ACE_INET_Addr control_addr(port, ip);
    ACE_SOCK_Connector connector;
    if (connector.connect(control_socket, control_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
        return 1;
    }
    return control_socket;
}

SOCK
loginToFtp(SOCK sock, Str user = "anonymous", Str pass = "")
{
    char buffer[1024];
    char comm[1024];
    ssize_t recv_count;

    // 接收服务器的欢迎信息
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 发送用户名和密码
    sprintf(comm, "USER %s\r\n", user);
    control_socket.send(comm, strlen(comm));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    sprintf(comm, "PASS %s\r\n", pass);
    control_socket.send(comm, strlen(comm));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
}

SOCK
connectAndLoginVimFtp()
{
    SOCK sock = connectToFtp("ftp.vim.org");
    loginToFtp(sock);
    return sock;
}

SOCK enterPassiveAndGetDataConnection(SOCK sock){
    char buffer[1024];
    ssize_t recv_count;


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

    return data_socket;
}

int getFtpFileSize(SOCK sock, const std::string& path);

void downloadOneSegment();

void connectLoginAndDownloadOneSegmentFromVim(SOCK sock, off_t off, size_t size) {

    SOCK sock = connectAndLoginVimFtp();
    SOCK dsock = enterPassiveAndGetDataConnection(sock);
}

void downloadOneSegment(SOCK csock, SOCK dsock, Str path, off_t off, size_t size){
        char buffer[1024];
    ssize_t recv_count;

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
}

// 定义控制连接的处理函数
void quitAndClose(ACE_SOCK_Stream &control_socket) {
    // 发送QUIT命令，关闭控制连接
    control_socket.send("QUIT\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;
    close(control_socket);
}

int run() {
    // 初始化ACE
    ACE::init();

    SOCK sock = connectToFtp("ftp.vim.org");

    // 处理控制连接void
    spawnDownloadsAndJoin(sock, 4);

    // 关闭ACE
    ACE::fini();

    return 0;
}

using SOCK = ACE_SOCK_Stream;

#include <thread>

/**
 * @brief Retrieves the size of a file from a given socket.
 *
 * This function sends a "SIZE" command to the socket and receives the response
 * containing the size of the file. The size is extracted from the response and
 * returned as an integer.
 *
 * @param sock The socket to communicate with.
 * @param path The path of the file.
 * @return The size of the file as an integer.
 *
 * @note This function assumes that the socket is already connected and the
 * necessary communication functions (e.g., `send`, `recv`) are available.
 * @note The format of the response is assumed to be "213 <size>", where <size>
 * is the size of the file.
 */
int
getFtpFileSize(SOCK sock, const std::string& path)
{
    char comm[1000];
    sprintf(comm, "SIZE %s", path.c_str()); // 发送SIZE
    sock.send(comm);
    char buffer[1000];
    sock.recv(buffer);
    int size;
    sscanf(buffer, "213 %d", &size); // 接收响应
    return size;
}

/**
 * @brief Spawns multiple threads to download a file in segments and joins them.
 *
 * This function divides the file into segments and creates multiple threads to
 * download each segment concurrently. After all the threads have completed
 * their tasks, this function waits for them to join before returning.
 *
 * @note This function assumes that the necessary variables and functions (e.g.,
 * `getSize`, `open`) are defined and initialized properly.
 *
 * @return void
 */
void
spawnDownloadsAndJoin(SOCK sock, Str path, int threads)
{
    std::set<std::thread> ts; // 计算大小，并且spawn若干线程以供下载。
    int fsize = getSize();
    int fhandle = open(path, O_RDWR);
    int segsize =
      static_cast<int>(static_cast<float>(fsize) / threads); // 向下取整

    for (int i = 0; i < threads - 1; i++) {
        int off = i * segsize;
        int len = segsize;
        ts.insert(std::thread(f, sock, off, len));

        std::cout << "Thread " << i << " Start" << std::endl;
    }

    int finaloff = (threads - 1) * segsize;
    ts.insert(std::thread(f, sock, finaloff, fsize - finaloff));
    std::cout << "Thread " << (threads - 1) << " Start" << std::endl;

    std::cout << "Download Complete" << std::endl;

    for (auto& t : ts) {
        t.join();
    }
}
