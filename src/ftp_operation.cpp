#include "ftp_operation.h"

#include "option.h"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using std::string;

int login_to_ftp(Ftp_Control_Client cli, string user, string pass)
{
    string c, t;

    // 接收服务器的欢迎信息
    if (cli.receive_reply(c, t)) {
        std::cout << "Error receiving welcome message from server."
                  << std::endl;
        return 1;
    }

    ACE_DEBUG((LM_DEBUG, "login welcome received: %s\n", t.c_str()));

    // 发送用户名
    if (cli.send_and_receive("USER", user, c, t)) {
        std::cout << "Error sending USER command to server." << std::endl;
        return 1;
    }
    // 判断返回的状态码是否正确
    if (c != "331") {
        return 1;
    }

    ACE_DEBUG((LM_DEBUG, "login user received: %s\n", t.c_str()));

    // 发送密码
    if (cli.send_and_receive("PASS", pass, c, t)) {
        std::cout << "Error sending PASS command to server." << std::endl;
        return 1;
    }
    // 判断返回的状态码是否正确
    if (c != "230") {
        return 1;
    }

    ACE_DEBUG((LM_DEBUG, "login pass received: %s\n", t.c_str()));

    return 0;
}

int enter_passive_and_get_data_connection(Ftp_Control_Client& cli, SOCK& dsock)
{
    ACE_TRACE(ACE_TEXT(__func__));
    string c, t;
    // 发送PASV命令，进入被动模式
    if (cli.send_and_receive("PASV", "", c, t)) {
        return 1;
    }
    ACE_DEBUG((LM_DEBUG, "pasv receive (%s, %s)", c.c_str(), t.c_str()));

    if (c != "227") {
        return 1;
    }

    // 解析被动模式响应，获取数据连接IP和端口号
    unsigned int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(t.c_str(), "Entering Passive Mode (%u,%u,%u,%u,%u,%u)", &ip1, &ip2,
           &ip3, &ip4, &port1, &port2);
    string data_ip = std::to_string(ip1) + "." + std::to_string(ip2) +
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
        SOCK sock,
        std::atomic<bool>& canceled)
{
    ACE_TRACE(ACE_TEXT(__func__));
    ACE_DEBUG(
            (LM_DEBUG,
             "%I%t enter passive and download: (off = %d, size = %u)\n", off,
             size));
    Ftp_Control_Client cli(sock);
    SOCK dsock;
    // 发送PASV，获取data socket。若发生错误，设置原子变量，通知各线程返回。
    if (enter_passive_and_get_data_connection(cli, dsock)) {
        canceled.store(true);
    }
    // 下载对应分段，出错则立即返回。错误信息已存储在原子变量中。
    if (download_one_segment(
                cli, dsock, path, off, size, part_id, file, canceled)) {
        return;
    }
    // 关闭数据连接。在关闭数据连接时出错，无需处理
    if (quit(cli)) {}
}

int download_one_segment(
        Ftp_Control_Client& cli,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file,
        std::atomic<bool>& canceled)
{
    ACE_TRACE(ACE_TEXT(__func__));
    char buffer[1024];
    ssize_t recv_count;
    string c, t;
    ssize_t total_received = 0;

    // 发送TYPE I命令，进入BINARY模式
    if (cli.send_and_receive("TYPE", "I", c, t)) {
        goto error;
    }
    ACE_DEBUG((LM_DEBUG, "%I%t sent TYPE I\n"));

    // 发送REST命令，设置下载的起始偏移量
    if (cli.send_and_receive("REST", std::to_string(start_offset), c, t)) {
        goto error;
    }
    ACE_DEBUG((LM_DEBUG, "%I%t sent REST\n"));

    // 发送RETR命令
    if (cli.send_and_receive("RETR", path, c, t)) {
        goto error;
    }
    if (c != "550") {
        std::cout << "Retrieve error, server sending bad return code.\n";
        // 错误处理，当错误发生时，所有线程立即取消
        canceled.store(true);
        goto error;
    }
    std::cout << "RETR is sent, start download from data socket.\n";

    // 下载文件
    ACE_DEBUG((LM_DEBUG, "%I%t part %d ready to get %u\n", part_id, size));
    while ((recv_count = data_socket.recv(buffer, sizeof(buffer))) > 0) {
        ACE_DEBUG(
                (LM_DEBUG, "%I%t part %d received %u\n", part_id, recv_count));
        // 错误处理，当错误发生时，所有线程立即取消
        if (canceled.load()) {
            goto error;
        }
        // 当socket错误发生时，用cancel_download原子变量通知别的线程返回
        if (recv_count == -1) {
            goto error;
        }
        ssize_t remaining = recv_count;
        if (total_received + remaining > size) {
            remaining = size - total_received;
        }
        ACE_OS::fwrite(buffer, 1, remaining, file);
        ACE_DEBUG((LM_DEBUG, "%I%t part %d wrote %u\n", part_id, remaining));
        total_received += remaining;
        if (total_received >= size) {
            break;
        }
    }

    std::cout << "Finish the " << part_id << " part. Totally have got "
              << total_received << "\n";

    // 关闭数据连接
    data_socket.close();

    return 0;

error:
    // 出错时在原子变量中存储状态，通知其他线程立即返回
    canceled.store(true);
    // 出错时也要关闭数据连接
    data_socket.close();
    return 1;
}

int quit(Ftp_Control_Client& cli)
{
    ACE_TRACE(ACE_TEXT(__func__));
    if (cli.send_command("QUIT", "")) {
        return 1;
    }
    return 0;
}

int get_ftp_file_size(
        Ftp_Control_Client cli,
        const string& path,
        int& result)
{
    ACE_TRACE(ACE_TEXT(__func__));
    string c, t;
    // 发送SIZE命令，获取文件大小
    if (cli.send_and_receive("SIZE", path, c, t)) {
        return 1;
    }
    ACE_DEBUG(
            (LM_DEBUG, "%I%t size query return (%s, %s)\n", c.c_str(),
             t.c_str()));
    // 检测返回码是否为213正确响应
    if (c != "213") {
        std::cout << "SIZE request received " << c << ", doesn't match 213\n";
        return 1;
    }
    result = std::stoi(t);
    return 0;
}

int fetch_nlst(
        Ftp_Control_Client cli,
        const string& cwd,
        string& result)
{
    static char buffer[1024];
    string c, t;

    result.clear();

    // 进入被动模式，获取data socket
    SOCK data_socket;
    if (enter_passive_and_get_data_connection(cli, data_socket)) {
        return 1;
    }

    // 发送NLST命令
    if (cli.send_and_receive("NLST", cwd, c, t)) {
        return 1;
    }

    // 检测返回码是否为226正确响应
    if (c != "226") {
        std::cout << "NLST request received " << c << ", doesn't match 226\n";
        return 1;
    }

    // 接收数据
    string received_lines;
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

int fetch_find_max(SOCK sock, string path, string& result)
{
    string s;
    if (fetch_nlst(sock, path, s)) {
        return 1;
    }
    std::cout << __func__ << " got nlst: " << s << std::endl;
    if (find_max(str_to_lines(s), result)) {
        return 1;
    }
    return 0;
}

int fetch_fzf(SOCK sock, string path, string e, VS& result)
{
    string s;
    if (fetch_nlst(sock, path, s)) {
        std::cout << "Fetching NLST failed with error code " << s << std::endl;
        return 1;
    }
    result = fzf(str_to_lines(s), e);
    return 0;
}

int fetch_find(SOCK sock, string path, string e, bool& result)
{
    string s;
    string e_with_prefix = path;
    join_path(e_with_prefix, e);
    if (fetch_nlst(sock, path, s)) {
        return 1;
    }
    std::cout << "NLST fetched to s\n";
    result = find(str_to_lines(s), e_with_prefix);
    return 0;
}

int Ftp_Control_Client::send_command(
        const string& command,
        const string& argument)
{
    string cmd = command + " " + argument;
    if (sock.send_line(cmd)) {
        return 1;
    }
    return 0;
}

int Ftp_Control_Client::receive_reply(
        string& status_code,
        string& result_lines)
{
    string line;
    result_lines.clear();
    while (true) {
        if (sock.receive_line(line)) {
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
        const string& command,
        const string& argument,
        string& status_code,
        string& result_lines)
{
    if (send_command(command, argument)) {
        return 1;
    }
    if (receive_reply(status_code, result_lines)) {
        return 1;
    }
    return 0;
}
