#include "ftp_util.h"
#include "ftp_operation.h"

#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/OS.h>
#include <ace/SOCK_Stream.h>
#include <iostream>

/**
 * @brief 输入Option，输出嗅探到的文件路径
 *
 * @return int 错误码，0是对的，其他是其他错误码
 */
int sniff(char* buffer, int size)
{
    if (size <= 200) {
        return 1;
    }
    if (strncpy(buffer,
                "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/debug/x86_64/"
                "8.0.35618/dbackup3-agent-8.0.35618-1.6586071.dbg.x86_64.rpm",
                size)) {
        return 2;
    }
    return 0;
}

void join_path(std::string& origin_, const std::string& appendix)
{
    if ((appendix.length()) == 0) {
        return;
    }

    if (origin_.back() != '/') {
        origin_.push_back('/');
    }

    if (appendix[0] == '/') {
        origin_ += (appendix.substr(1));
    } else {
        origin_ += appendix;
    }

    return;
}

int get_regular_name(string path, string& result)
{
    if (path.back() == '/') {
        path.pop_back();
    }
    size_t pos = path.rfind("/");
    if (pos == string::npos) {
        result = path;
    }
    result = path.substr(pos + 1);
    return 0;
}

std::vector<std::string> str_to_lines(string text)
{
    std::vector<std::string> result;
    std::istringstream iss(text);
    std::string line;

    while (std::getline(iss, line, '\n')) {
        if (line.length() >= 1 && line.back() == '\r') {
            line.pop_back();
        }
        result.push_back(line);
    }
    return result;
}

/**
 * @brief 找是否出现过
 *
 * @param v
 * @param e
 * @return true
 * @return false
 */
bool find(vector<string> v, std::string e)
{
    return std::find(v.begin(), v.end(), e) != v.end();
}

/**
 * @brief fuzzy find.
 *
 * @param ss
 * @param e
 * @return VS
 */
VS fzf(VS ss, string e)
{
    VS result;
    for (auto s : ss) {
        if (s.find(e) != string::npos) {
            result.emplace_back(s);
        }
    }
    return result;
}

// 函数用于找最大版本号的辅助函数
int find_max(const VS& ss, std::string& result)
{
    std::string line;
    std::tuple<int, int, int> max_number{0, 0, 0};
    if (ss.size() < 1)
        return 1;

    for (auto line : ss) {
        std::string r_name;
        get_regular_name(line, r_name);
        std::cout << "Processing r_name: " << r_name << "\n";
        // 将数字解析为tuple
        int a, b, c;
        char dot;
        sscanf(r_name.c_str(), "%d.%d.%d", &a, &b, &c);
        std::tuple<int, int, int> current_number{a, b, c};
        if (max_number < current_number) {
            max_number = current_number;
            result = r_name;
        }
    }
    return 0;
}

/**
 * @brief 获取文件列表并查找最大值
 *
 * 该函数通过与服务器建立的套接字sock，获取指定路径path下的文件列表，
 * 并在文件列表中查找最大值，并将结果存储在给定的result字符串中。
 *
 * @param sock 与服务器建立的套接字。
 * @param path 指定的路径。
 * @param result 存储最大值的字符串。
 * @return 如果成功获取并查找最大值，则返回0；否则返回非零值。
 */
int fetch_find_max(SOCK sock, Str path, Str& result)
{
    Str s;
    fetch_nlst(sock, path, s);
    std::cout << __func__ << " got nlst: " << s << std::endl;
    find_max(str_to_lines(s), result);
    return 0;
}

/**
 * @brief 获取文件列表并使用FZF过滤
 *
 * 该函数通过与服务器建立的套接字sock，获取指定路径path下的文件列表，
 * 并使用FZF过滤出匹配给定模式e的文件列表。
 *
 * @param sock 与服务器建立的套接字。
 * @param path 指定的路径。
 * @param e 过滤模式。
 * @param result 存储过滤后的文件列表的向量。
 * @return 如果成功获取并过滤文件列表，则返回0；否则返回非零值。
 */
int fetch_fzf(SOCK sock, Str path, Str e, VS& result)
{
    Str s;
    if (fetch_nlst(sock, path, s)) {
        std::cout << "fetch_nlst failed with code " << s << std::endl;
        return 1;
    }
    result = fzf(str_to_lines(s), e);
    return 0;
}

/**
 * @brief 从指定的 SOCK 连接中获取文件列表，并查找指定的文件路径。
 *
 * @param sock 连接的 SOCK 对象。
 * @param path 文件路径。
 * @param e 文件名。
 * @param result [out] 查找结果，true 表示找到，false 表示未找到。
 * @return 返回操作的结果，0 表示成功，非零值表示失败。
 */
int fetch_find(SOCK sock, Str path, Str e, bool& result)
{
    Str s;
    Str e_with_prefix = path;
    join_path(e_with_prefix, e);
    if (fetch_nlst(sock, path, s)) {
        return 1;
    }
    std::cout << "NLST fetched to s\n";
    result = find(str_to_lines(s), e_with_prefix);
    return 0;
}

bool fetch_exist(SOCK sock, Str path)
{
    Str s;
    fetch_nlst(sock, path, s);
    return str_to_lines(s).size() == 1;
}

char buffer[1024];
ssize_t recv_count;

/**
 * FTP登录与转入被动
 */
void setup_control(ACE_SOCK_Stream& control_socket)
{
    // current working directory
    std::string cwd = "/ftp_product_installer/dbackup3/rpm";

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
}

/**
 * @brief 抓取目录列表
 *
 * @param control_socket
 * @param cwd
 * @param data_ip
 * @param data_port
 * @param result
 */
int fetch_nlst(
        ACE_SOCK_Stream& control_socket,
        const std::string& cwd,
        std::string& result)
{
    std::fill(buffer, buffer + sizeof(buffer), '\0');
    SOCK data_socket;
    enter_passive_and_get_data_connection(control_socket, data_socket);

    // 发送NLST命令
    ACE_OS::sprintf(buffer, "NLST %s\r\n", cwd.c_str());
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    // recv_count = control_socket.recv(buffer, sizeof(buffer));
    // buffer[recv_count] = '\0';
    // std::cout << buffer;

    sleep(1);

    // 接收数据
    std::string received_lines;
    while ((data_socket.recv(buffer, sizeof(buffer))) > 0) {
        received_lines += buffer;
    }

    // 关闭数据连接
    data_socket.close();

    // 接收NLST命令响应
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 结束流程并返回值
    result = received_lines;
    return 0;
}
std::string receiveLine(
        ACE_SOCK_Stream& socket,
        ACE_Message_Block& message_block)
{
    std::string line;

    while (true) {
        ssize_t bytesRead =
                socket.recv(message_block.wr_ptr(), message_block.space());

        if (bytesRead <= 0) {
            // 出现错误或连接关闭
            ACE_DEBUG((LM_ERROR, "Error receiving data from socket.\n"));
            break;
        }

        message_block.wr_ptr(bytesRead);

        char* newlinePos = std::find(
                message_block.rd_ptr(), message_block.wr_ptr(), '\n');

        if (newlinePos != message_block.wr_ptr()) {
            // 找到换行符，提取一行数据
            ssize_t lineLength = newlinePos - message_block.rd_ptr() + 1;
            line.append(message_block.rd_ptr(), lineLength);

            // 移动消息块的读指针
            message_block.rd_ptr(lineLength);

            break;
        }

        // 没有找到换行符，继续接收数据
        // messageBlock.expand(1024); // 扩展消息块的大小
    }

    return line;
}

using MB = ACE_Message_Block;

/**
 * @brief 为FTP控制连接通信而设计的Socket封装
 *
 * 为FTP控制连接通信设计的Socket封装，能够按行接收响应
 * 以便更好判断返回状态
 *
 */
class Lined_Sock
{
    SOCK sock;
    MB block{1024};

    Lined_Sock(SOCK sock_): sock(sock_) {}
    int get_line(Str& result)
    {
        result = receiveLine(sock, block);
        return 0;
    }
};

int get_status_code(const char* line)
{
    if (strlen(line) < 3)
        return -1;
    char buf[4];
    strncpy(buf, line, 3);
    return atoi(buf);
}

int get_status_code(Str line)
{
    int i = std::atoi(line.substr(0, 3).c_str());
    return i;
}

Sock_Creator make_logined_sock_creator(
        const ACE_INET_Addr& ftp_address,
        const std::string& username,
        const std::string& password)
{
    return [=](SOCK& sock) {
        // 连接到对端地址
        ACE_SOCK_Connector connector;
        if (connector.connect(sock, ftp_address) == -1) {
            std::cerr << "Failed to connect to FTP server." << std::endl;
            return 1;
        }
        // 登录到FTP服务器
        if (login_to_ftp(sock, username, password)) {
            std::cerr << "Failed to login to FTP server." << std::endl;
            sock.close();
            return 1;
        }
        return 0;
    };
}
