#include "option.h"
#include "ftp_operation.h"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>

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
SOCK connect_to_ftp(string ip, int port)
{
    // 建立控制连接
    ACE_SOCK_Stream sock;
    ACE_INET_Addr control_addr(port, ip.c_str());
    ACE_SOCK_Connector connector;
    if (connector.connect(sock, control_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
        return 1;
    }
    return sock;
}

/**
 * @brief 登录到FTP服务器。
 *
 * 此函数向FTP服务器发送用户名和密码以进行登录。
 *
 * @param sock 控制连接的套接字。
 * @param user FTP服务器的用户名。
 * @param pass FTP服务器的密码。
 * @return 如果成功登录，则返回0；如果出现错误，则返回1。
 */
int login_to_ftp(Ftp_Control_Client cli, string user, string pass)
{
    string c, t;

    // 接收服务器的欢迎信息
    if (cli.receive_reply(c, t)) {
        std::cout << "Error receiving welcome message from server."
                  << std::endl;
        return 1;
    }

    std::cout << "login welcome received: " << t << std::endl;

    // 发送用户名和密码
    if (cli.send_and_receive("USER", user, c, t)) {
        std::cout << "Error sending USER command to server." << std::endl;
        return 1;
    }

    std::cout << "login user received: " << t << std::endl;

    if (cli.send_and_receive("PASS", pass, c, t)) {
        std::cout << "Error sending PASS command to server." << std::endl;
        return 1;
    }

    std::cout << "login pass received: " << t << std::endl;

    return 0;
}

/**
 * @brief 进入被动模式并建立数据连接。
 *
 * 此函数向FTP服务器发送PASV命令以进入被动模式，并解析响应以获取数据连接的IP地址和端口号。
 * 然后，它使用获取的IP地址和端口号建立数据连接。
 *
 * @param sock 控制连接的套接字。
 * @param dsock 数据连接的套接字。
 * @return 如果成功建立数据连接，则返回0；如果出现错误，则返回1。
 */
int enter_passive_and_get_data_connection(Ftp_Control_Client cli, SOCK& dsock)
{
    string c, t;
    std::cout << "to pasv\n";
    // 发送PASV命令，进入被动模式
    cli.send_command("PASV", "");
    cli.receive_reply(c, t);
    std::cout << "got pasv" << t << "\n";

    if (c != "227") {
        return 1;
    }

    // 解析被动模式响应，获取数据连接IP和端口号
    unsigned int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(t.c_str(), "Entering Passive Mode (%u,%u,%u,%u,%u,%u)", &ip1, &ip2,
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
    // std::cout << "sleeping\n";
    // sleep(2);
    // std::cout << "awake\n";
    if (connector.connect(data_socket, data_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
        exit(0);
        return 1;
    }

    dsock = data_socket;
    return 0;
}

/**
 * @brief 进入被动模式，下载指定片段的文件，并关闭连接。
 *
 * @param path 文件路径。
 * @param off 文件偏移量。
 * @param size 文件大小。
 * @param part_id 片段 ID。
 * @param file 文件指针。
 * @param sock 连接的 SOCK 对象。
 */
void enter_passive_and_download_one_segment_and_close(
        string path,
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
 * @param sock 用于向FTP服务器发送命令的控制套接字。
 * @param data_socket 用于接收文件数据的数据套接字。
 * @param path FTP服务器上文件的路径。
 * @param start_offset 要下载的部分的起始偏移量。
 * @param size 要下载的部分的大小。
 * @param part_id 正在下载的部分的ID。
 *
 * @note 此函数假设控制套接字和数据套接字已经连接到FTP服务器。
 */
void download_one_segment(
        Ftp_Control_Client cli,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file)
{
    char buffer[1024];
    ssize_t recv_count;
    string c, t;

    // 发送TYPE I命令，进入BINARY模式
    cli.send_command("TYPE", "I");
    cli.receive_reply(c, t);
    std::cout << "Sent TYPE I\n";

    // 发送REST命令，设置下载的起始偏移量
    cli.send_command("REST", std::to_string(start_offset));
    cli.receive_reply(c, t);

    // 发送RETR命令
    cli.send_command("RETR", path);
    cli.receive_reply(c, t);
    if (c != "550") {
        std::cout << "open error\n";
        return;
    }
    std::cout << "sent retr\n";

    // 下载文件
    std::cout << "The " << part_id << " ready to get " << size << "\n";
    ssize_t total_received = 0;
    while ((recv_count = data_socket.recv(buffer, sizeof(buffer))) > 0) {
        std::cout << "The " << part_id << " received " << recv_count << "\n";
        ssize_t remaining = recv_count;
        if (total_received + remaining > size) {
            remaining = size - total_received;
        }
        ACE_OS::fwrite(buffer, 1, remaining, file);
        std::cout << "The " << part_id << " wrote " << remaining << "\n";
        total_received += remaining;
        if (total_received >= size) {
            break;
        }
    }

    std::cout << "The " << part_id << " totally have got " << total_received
              << "\n";

    // 关闭数据连接
    data_socket.close();

    // 接收RETR命令响应
    cli.receive_reply(c, t);
}

/**
 * @brief 关闭控制连接并退出FTP服务器。
 *
 * 此函数发送QUIT命令关闭控制连接，并从FTP服务器退出。
 *
 * @param sock 控制连接的套接字。
 * @return 返回值为0表示成功关闭控制连接和退出FTP服务器，否则表示出现错误。
 */
int quit_and_close(SOCK& sock)
{
    Ftp_Control_Client cli(sock);
    cli.send_command("QUIT", "");

    sock.close();
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
int get_ftp_file_size(Ftp_Control_Client cli, const std::string& path)
{
    std::cout << "Getting Ftp Size\n";

    string c, t;

    cli.send_command("SIZE", path);

    cli.receive_reply(c, t);

    std::cout << "recieved size. size is " << t << "\n";

    if (c != "213") {
        return -1;
    }

    return std::stoi(t);
}

int Ftp_Control_Client::send_command(
        const std::string& command,
        const std::string& argument)
{
    std::string cmd = command + " " + argument;
    if (sock.sendLine(cmd)) {
        return 1;
    }
    return 0;
}

int Ftp_Control_Client::receive_reply(
        std::string& status_code,
        std::string& result_lines)
{
    std::string line;
    result_lines.clear();
    while (true) {
        if (sock.receiveLine(line)) {
            // 接收失败或连接关闭
            return 1;
        }

        // 检查行是否符合Completion
        // reply模式（2xx或4xx）
        // 1. 是Completion Reply且是中间状态行（状态码后接'-'字符）
        if (line.size() >= 4 && isdigit(line[0]) && isdigit(line[1]) &&
            isdigit(line[2]) && line[3] == '-') {
            status_code = line.substr(0, 3);
            result_lines += (line.substr(4) + "\n");
            continue;
        }
        // 2. 是Completion Reply且不是中间状态行
        else if (
                line.size() >= 4 && isdigit(line[0]) && isdigit(line[1]) &&
                isdigit(line[2]) && line[3] == ' ') {
            result_lines += (line.substr(4) + "\n");
            return 0;
        }
        // 3. 是中间状态行
        else {
            continue;
        }
    }
}

int Ftp_Control_Client::send_and_receive(
        const std::string& command,
        const std::string& argument,
        std::string& status_code,
        std::string& result_lines)
{
    if (send_command(command, argument)) {
        return 1;
    }
    if (receive_reply(status_code, result_lines)) {
        return 1;
    }
    return 0;
}
