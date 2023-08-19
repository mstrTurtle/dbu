#include "Sniffer.h"
#include "cstring"
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <boost/filesystem.hpp>


/**
 * @brief 输入Option，输出嗅探到的文件路径
 *
 * @return int 错误码，0是对的，其他是其他错误码
 */
int
sniff(char* buffer, int size)
{
  if (size <= 200) {
    return 1;
  }
  if (strncpy(
        buffer,
        "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/debug/x86_64/"
        "8.0.35618/dbackup3-agent-8.0.35618-1.6586071.dbg.x86_64.rpm",
        size)) {
    return 2;
  }
  return 0;
}

std::string& join_path(std::string& origin_, const std::string& appendix)
{
    if ((appendix.length()) == 0) {
        return origin_;
    }

    if (origin_.back() != '/') {
        origin_.push_back('/');
    }

    if (appendix[0] == '/') {
        origin_ += (appendix.substr(1));
    } else {
        origin_ += appendix;
    }

    return origin_;
}

// 函数用于按行检测是否包含某子串，并将包含子串的行添加到vector中
void
findAndAddLines(const std::string& input,
                const std::string& substring,
                std::vector<std::string>& result)
{
    size_t pos = 0;

    for (;;) {
        pos = input.find(substring, pos);

        if (pos != std::string::npos) {
          return;
        }

        size_t beg, end;
        end = input.find("\r\n", pos);
        if (end == std::string::npos) {
          end = input.length();
        }
        beg = input.find("\r\n", pos);
        if (beg == std::string::npos) {
          beg = 0;
        }
        result.emplace_back(input.substr(beg, end - beg));
    }
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

char buffer[1024];
ssize_t recv_count;

/**
 * 建立FTP Control连接的过程
 */
void setupControl(ACE_SOCK_Stream &control_socket, std::string& data_ip, u_short& data_port){

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

    // 发送PASV命令，进入被动模式
    control_socket.send("PASV\r\n", 6);
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

    // 解析被动模式响应，获取数据连接IP和端口号
    unsigned int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(buffer, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    data_ip = std::to_string(ip1) + "." + std::to_string(ip2) + "." +
                          std::to_string(ip3) + "." + std::to_string(ip4);
    data_port = (port1 << 8) + port2;

}
// 函数用于将以"\r\n"相间隔的字符串转换为vector<string>
std::vector<std::string> convertToVector(const std::string& text) {
    std::vector<std::string> result;
    std::istringstream iss(text);
    std::string line;

    while (std::getline(iss, line, '\r')) {
        std::istringstream lineStream(line);
        std::string item;

        while (std::getline(lineStream, item, '\n')) {
            result.push_back(item);
        }
    }

    return result;
}

// 函数用于过滤包含指定子字符串的项
std::vector<std::string> filterBySubstring(const std::vector<std::string>& items, const std::string& substring) {
    std::vector<std::string> filteredItems;

    for (const std::string& item : items) {
        if (item.find(substring) != std::string::npos) {
            filteredItems.push_back(item);
        }
    }

    return filteredItems;
}

void fetchNLST(ACE_SOCK_Stream &control_socket,const std::string& cwd,
        const std::string& data_ip, u_short data_port, std::string& result){

    // 建立数据连接
    ACE_SOCK_Stream data_socket;
    ACE_INET_Addr data_addr(data_port, data_ip.c_str());
    ACE_SOCK_Connector connector;
    if (connector.connect(data_socket, data_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
        return;
    }

    // 发送NLST命令
    ACE_OS::sprintf(buffer, "NLST %s\r\n", cwd.c_str());
    control_socket.send(buffer, ACE_OS::strlen(buffer));
    recv_count = control_socket.recv(buffer, sizeof(buffer));
    buffer[recv_count] = '\0';
    std::cout << buffer;

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

    return received_lines;
}

// 函数用于在以"\r\n"分隔的文本中找到排序最大的数字
int findMaxNumber(const std::vector<std::string>& lines, std::string& result) {
    std::istringstream iss(text);
    std::string line;
    std::tuple<int, int, int> maxNumber(0, 0, 0);

    if(lines.length() < 1)
        return 1;

    for(auto line:lines){
        // 将数字解析为tuple
        int a, b, c;
        char dot;
        std::istringstream numberStream(number);
        sscanf(line.c_str(), "%d.%d.%d", &a, &b, &c);
        std::tuple<int, int, int> currentNumber(a, b, c);
        if(maxNumber< currentNumber){
            maxNumber = currentNumber;
            result = line;
        }
    }
    return 0;
}

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// 函数用于判断主分支并处理分支
int processBranch(const std::string& cwd, const std::string& branch, const std::string& subbranch) {
    // 1. 判断主分支. dev, master没有次分支。
    if (branch == "develop" || branch == "master") {
        // 有些分支不用嗅探
        std::string newCwd = path_join(cwd, "develop");
        // 处理新的路径
        // ...
    } else if (branch == "feature" || branch == "hotfix" || branch == "support") {
        // 继续
        std::vector<std::string> received_lines;
        fetchNLST(control_socket, cwd, received_lines);
        std::vector<std::string> v = convertToVector(received_lines);
        std::vector<std::string> v1 = filterBySubstring(v, subbranch);
        if (v1.size() == 0) {
            return 1;
        } else if (v1.size() == 1) {
            std::string newCwd = path_join(cwd, v1[0]);
            // 处理新的路径
            // ...
            return 0;
        } else {
            std::cout << "subbranch error, too many possibles" << std::endl;
            return 1;
        }
    } else {
        return 1;
    }
}

int processOption(cwd, control_socket){
    fetchNLST(control_socket,cwd,received_lines);
    v = convertToVector(received_lines);
    if(v.length()==1){

    }
    else if (v.length()==0){

    }
    else{
        findMaxNumber(v, result);
    }
}
int processTarget(cwd){
    fetchNLST(control_socket,cwd,received_lines);
    v = convertToVector(received_lines);
    if(v.length()==1){

    }
    else if (v.length()==0){

    }
    else{
        findMaxNumber(v, result);
    }
}
int processVersion(cwd){
    fetchNLST(control_socket,cwd,received_lines);
    v = convertToVector(received_lines);
    if(v.length()==1){

    }
    else if (v.length()==0){

    }
    else{
        findMaxNumber(v, result);
    }
}
int processFunctionality(cwd){
    fetchNLST(control_socket,cwd,received_lines);
    v = convertToVector(received_lines);
    if(v.length()==1){

    }
    else if (v.length()==0){

    }
    else{
        findMaxNumber(v, result);
    }
}

/**
 * 嗅探的入口。传入Option，逐步嗅探。返回错误码。
 * TODO: 我觉得得定义一下错误码。
 */
void sniffRun(ACE_SOCK_Stream &control_socket, std::string& result, Option option){
    std::string data_ip;
    u_short data_port;

    std::string cwd = "/ftp_product_installer/dbackup3/rpm";

    setupControl(control_socket,data_ip,data_port);

    if(ret=processBranch()
        |processOption()
        |processTarget()
        |processVersion()
        |processFunctionality(){
        // print error
        exit(ret);
    }

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

