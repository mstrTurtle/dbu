#include "ftp_util.h"

#include "ftp_operation.h"
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/OS.h>
#include <ace/SOCK_Stream.h>
#include <iostream>

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

int find_max(const VS& ss, std::string& result)
{
    std::string line;
    std::tuple<int, int, int> max_number{0, 0, 0};
    if (ss.size() < 1)
        return 1;

    for (auto line : ss) {
        std::string r_name;
        if (get_regular_name(line, r_name)) {
            return 1;
        }
        ACE_DEBUG((LM_DEBUG, "Processing r_name: %s\n", r_name.c_str()));
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

Sock_Creator make_logined_sock_creator(
        const ACE_INET_Addr& ftp_address,
        const std::string& username,
        const std::string& password)
{
    return [=](SOCK& sock) {
        // 连接到对端地址
        ACE_SOCK_Connector connector;
        if (connector.connect(sock, ftp_address) == -1) {
            std::cout << "Failed to connect to FTP server." << std::endl;
            return 1;
        }
        // 登录到FTP服务器
        if (login_to_ftp(sock, username, password)) {
            std::cout << "Failed to login to FTP server." << std::endl;
            sock.close();
            return 1;
        }
        return 0;
    };
}

int Lined_SOCK::send_line(const std::string& line)
{
    std::string message = line + "\r\n";
    ssize_t bytes_sent = sock.send_n(message.c_str(), message.length());
    if (bytes_sent != static_cast<ssize_t>(message.length())) {
        return 1;
    } else
        return 0;
}

int Lined_SOCK::receive_line(std::string& line)
{
    line.clear();

    while (true) {
        // 检查缓冲区中是否有剩余数据
        size_t newline_pos = buffer.find("\r\n");
        if (newline_pos != std::string::npos) {
            // 从缓冲区中提取行
            line = buffer.substr(0, newline_pos);

            // 从缓冲区中删除行（包括分隔符）
            buffer.erase(0, newline_pos + 2);

            return 0;
        }

        // 从套接字中读取更多数据到缓冲区
        char recv_buffer[1024];
        ssize_t bytes_read = sock.recv(recv_buffer, sizeof(recv_buffer));
        if (bytes_read <= 0) {
            // 错误或连接关闭
            return 1;
        }

        // 将接收到的数据追加到缓冲区
        buffer.append(recv_buffer, bytes_read);
    }
}
