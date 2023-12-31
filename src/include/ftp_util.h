/**
 * @file ftp_util.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief FTP实用工具，包括字符串检索操作、ACE_SOCK_Stream浅封装等。
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once

#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <ace/Message_Block.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using VS = std::vector<std::string>;
using SOCK = ACE_SOCK_Stream;

/**
 * @brief 将字符串追加到原始字符串的末尾，形成新的路径
 *
 * @param origin_ 原始字符串
 * @param appendix 追加的字符串
 */
void join_path(std::string& origin_, const std::string& appendix);

/**
 * @brief 将字符串按行拆分为字符串向量
 *
 * @param text 输入的文本字符串
 * @return std::vector<std::string> 拆分后的字符串向量
 */
[[nodiscard]] std::vector<std::string> str_to_lines(std::string text);

/**
 * @brief 在字符串向量中查找指定字符串
 *
 * @param v 字符串向量
 * @param e 指定字符串
 * @return bool 如果找到指定字符串，则返回true；否则返回false
 */
[[nodiscard]] bool find(std::vector<std::string> v, std::string e);

/**
 * @brief 模糊查找字符串向量中包含指定字符串的字符串
 *
 * @param ss 字符串向量
 * @param e 指定字符串
 * @return VS 包含指定字符串的字符串向量
 */
[[nodiscard]] VS fzf(VS ss, std::string e);

/**
 * @brief 在字符串向量中查找具有最大版本号的字符串
 *
 * @param ss 字符串向量
 * @param result 存储具有最大版本号的字符串
 * @return int 错误码，0表示成功，其他表示错误
 */
[[nodiscard]] int find_max(const VS& ss, std::string& result);

/**
 * @brief 从给定路径中提取文件名
 *
 * @param path 路径字符串
 * @param result 存储提取的文件名
 * @return int 错误码，0表示成功，其他表示错误
 */
[[nodiscard]] int get_regular_name(std::string path, std::string& result);

/**
 * @brief 模糊查找字符串向量中包含指定字符串的字符串
 *
 * @param ss 字符串向量
 * @param e 指定字符串
 * @return VS 包含指定字符串的字符串向量
 */
[[nodiscard]] VS fzf(VS ss, std::string e);

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

    Lined_SOCK(const Lined_SOCK& other) = delete;

    Lined_SOCK(Lined_SOCK&& other): sock(other.sock)
    {
        buffer = other.buffer;
        other.buffer.clear();
    }

    Lined_SOCK& operator=(const Lined_SOCK& other) = delete;

    Lined_SOCK& operator=(Lined_SOCK&& other)
    {
        sock = other.sock;
        buffer = other.buffer;
        other.buffer.clear();
        return *this;
    }

    ~Lined_SOCK()
    {
        sock.close();
    }

    /**
     * @brief 发送一行文本。
     * @param line 要发送的文本行
     * @return 发送成功返回0，发送失败返回1
     */
    [[nodiscard]] int send_line(const std::string& line);

    /**
     * @brief 接收一行文本。
     * @param line 存储接收到的文本行
     * @return 接收成功返回0，接收失败返回1
     */
    [[nodiscard]] int receive_line(std::string& line);
};
