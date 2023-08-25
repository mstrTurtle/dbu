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
std::vector<std::string> str_to_lines(string text);

/**
 * @brief 在字符串向量中查找指定字符串
 *
 * @param v 字符串向量
 * @param e 指定字符串
 * @return bool 如果找到指定字符串，则返回true；否则返回false
 */
bool find(vector<string> v, std::string e);

/**
 * @brief 模糊查找字符串向量中包含指定字符串的字符串
 *
 * @param ss 字符串向量
 * @param e 指定字符串
 * @return VS 包含指定字符串的字符串向量
 */
VS fzf(VS ss, string e);

/**
 * @brief 在字符串向量中查找具有最大版本号的字符串
 *
 * @param ss 字符串向量
 * @param result 存储具有最大版本号的字符串
 * @return int 错误码，0表示成功，其他表示错误
 */
int find_max(const VS& ss, std::string& result);

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
int fetch_find_max(SOCK sock, string path, string& result);

/**
 * @brief 获取文件列表并使用FZF过滤
 *
 * 该函数通过与服务器建立的套接字sock，获取指定路径path下的文件列表，
 * 并使用模糊匹配过滤出匹配给定模式e的文件列表。
 *
 * @param sock 与服务器建立的套接字。
 * @param path 指定的路径。
 * @param e 过滤模式。
 * @param result 存储过滤后的文件列表的向量。
 * @return 如果成功获取并过滤文件列表，则返回0；否则返回非零值。
 */
int fetch_fzf(SOCK sock, string path, string e, VS& result);


/**
 * @brief 从指定的 SOCK 连接中获取文件列表，并查找指定的文件路径。
 *
 * @param sock 连接的 SOCK 对象。
 * @param path 文件路径。
 * @param e 文件名。
 * @param result [out] 查找结果，true 表示找到，false 表示未找到。
 * @return 返回操作的结果，0 表示成功，非零值表示失败。
 */
int fetch_find(SOCK sock, string path, string e, bool& result);

/**
 * @brief 从给定路径中提取文件名
 *
 * @param path 路径字符串
 * @param result 存储提取的文件名
 * @return int 错误码，0表示成功，其他表示错误
 */
int get_regular_name(string path, string& result);

/**
 * @brief 模糊查找字符串向量中包含指定字符串的字符串
 *
 * @param ss 字符串向量
 * @param e 指定字符串
 * @return VS 包含指定字符串的字符串向量
 */
VS fzf(VS ss, string e);

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
