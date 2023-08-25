#pragma once

#include <cstring>
#include <algorithm>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/Message_Block.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>

using std::string;
using std::vector;

using VS = vector<string>;

using SOCK = ACE_SOCK_Stream;

int fetch_nlst(
        ACE_SOCK_Stream& sock,
        const std::string& cwd,
        std::string& result);

int sniff(char* buffer, int size);

void join_path(std::string& origin_, const std::string& appendix);

std::vector<std::string> str_to_lines(string text);

VS fzf(VS ss, string e);

int find_max(const VS& ss, std::string& result);

int fetch_find_max(SOCK sock, string path, string& result);

int fetch_fzf(SOCK sock, string path, string e, VS& result);

int fetch_find(SOCK sock, string path, string e, bool& result);

bool fetch_exist(SOCK sock, string path);

void setup_control(ACE_SOCK_Stream& sock);

int get_status_code(const char* line);

int get_status_code(string line);

int get_regular_name(string path, string& result);

/**
 * @class Lined_SOCK
 * @brief 用于在ACE_SOCK_Stream上发送和接收以换行符分隔的文本行的封装类。
 */
class Lined_SOCK
{
private:
    ACE_SOCK_Stream sock; ///< ACE_SOCK_Stream对象
    std::string buffer;   ///< 缓冲区用于存储接收到的数据

public:
    /**
     * @brief 构造函数，使用给定的SOCK对象初始化Lined_SOCK。
     * @param sock_ ACE_SOCK_Stream对象
     */
    Lined_SOCK(const SOCK& sock_): sock(sock_) {}

    /**
     * @brief 发送一行文本。
     * @param line 要发送的文本行
     * @return 发送成功返回0，发送失败返回1
     */
    int sendLine(const std::string& line);

    /**
     * @brief 接收一行文本。
     * @param line 存储接收到的文本行
     * @return 接收成功返回0，接收失败返回1
     */
    int receiveLine(std::string& line);
};
