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

int enter_passive_and_get_data_connection(Ftp_Control_Client cli, SOCK& dsock)
{
    std::cout << "In" << __func__ << "\n";
    string c, t;
    // 发送PASV命令，进入被动模式
    cli.send_command("PASV", "");
    cli.receive_reply(c, t);
    std::cout << "got pasv reply: " << t << "\n";

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
    if (connector.connect(data_socket, data_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
        return 1;
    }

    dsock = data_socket;
    return 0;
}

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
    cli.send_and_receive("TYPE", "I", c, t);
    std::cout << "Sent TYPE I\n";

    // 发送REST命令，设置下载的起始偏移量
    cli.send_and_receive("REST", std::to_string(start_offset), c, t);

    // 发送RETR命令
    cli.send_and_receive("RETR", path, c, t);
    if (c != "550") {
        std::cout << "open error\n";
        return;
    }
    std::cout << "RETR is sent, start download from data socket.\n";

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

int quit_and_close(SOCK& sock)
{
    Ftp_Control_Client cli(sock);
    cli.send_command("QUIT", "");

    sock.close();
    return 0;
}

int get_ftp_file_size(Ftp_Control_Client cli, const std::string& path)
{
    std::cout << "Getting Ftp Size\n";

    string c, t;

    cli.send_and_receive("SIZE", path, c, t);

    std::cout << "recieved size. size is " << t << "\n";

    if (c != "213") {
        return -1;
    }

    return std::stoi(t);
}

int fetch_nlst(
        Ftp_Control_Client cli,
        const std::string& cwd,
        std::string& result)
{
    static char buffer[1024];
    string c, t;

    result.clear();

    // 进入被动模式，获取data socket
    SOCK data_socket;
    enter_passive_and_get_data_connection(cli, data_socket);

    // 发送NLST命令
    cli.send_and_receive("NLST", cwd, c, t);

    // 接收数据
    std::string received_lines;
    int recv_count;
    while ((recv_count = data_socket.recv(buffer, sizeof(buffer))) > 0) {
        buffer[recv_count] = '\0';
        result += buffer;
    }

    // 关闭数据连接
    data_socket.close();

    // 结束流程并返回值
    return 0;
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
