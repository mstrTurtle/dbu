/**
 * @file sniffer.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 根据提示探测资源路径的类。
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "ftp_operation.h"
#include "ftp_util.h"
#include "option.h"
#include "sniffer.h"
#include "cstring"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using std::vector;
using std::string;

using VS = vector<string>;
/**
 * @brief Sniff_Hint结构体用于存储探测提示信息
 */
struct Sniff_Hint
{
    const string branch;    /**< 分支 */
    const string subbranch; /**< 子分支 */
    const string option;    /**< 选项 */
    const string arch;      /**< 架构 */
    const string product;   /**< 产品 */

    /**
     * @brief 重载输出流操作符，用于打印Sniff_Hint对象的信息
     * @param os 输出流
     * @param hint Sniff_Hint对象
     * @return std::ostream& 输出流
     */
    friend std::ostream& operator<<(std::ostream& os, const Sniff_Hint& hint)
    {
        os << "Branch: " << hint.branch << std::endl;
        os << "Subbranch: " << hint.subbranch << std::endl;
        os << "Option: " << hint.option << std::endl;
        os << "Arch: " << hint.arch << std::endl;
        os << "Product: " << hint.product << std::endl;
        return os;
    }
};

/**
 * @brief Sniffer 类用于探测目标路径
 */
class Sniffer final
{
private:
    struct
    {
        ACE_INET_Addr addr; /**< 地址信息 */
        SOCK sock;          /**< socket 信息 */
    } conn;
    Sniff_Hint hint; /**< 探测提示信息 */
    string cwd;      /**< 当前工作路径 */

    /**
     * @brief 处理分支信息
     *
     * @return int 错误码，0 表示成功，其他值表示失败
     */
    [[nodiscard]] int process_branch();

    /**
     * @brief 处理选项信息
     *
     * @return int 错误码，0 表示成功，其他值表示失败
     */
    [[nodiscard]] int process_option();

    /**
     * @brief 处理目标信息
     *
     * @return int 错误码，0 表示成功，其他值表示失败
     */
    [[nodiscard]] int process_target();

    /**
     * @brief 处理版本号信息
     *
     * @return int 错误码，0 表示成功，其他值表示失败
     */
    [[nodiscard]] int process_version();

    /**
     * @brief 处理功能信息
     *
     * @return int 错误码，0 表示成功，其他值表示失败
     */
    [[nodiscard]] int process_functionality();

public:
    Sniffer() = delete;
    Sniffer(ACE_INET_Addr addr_, SOCK sock_, Sniff_Hint hint_)
        : conn({.addr = addr_, .sock = sock_}),
          hint(hint_){};

    /**
     * @brief 探测的入口。传入Option，逐步探测。返回错误码。
     *
     * @param result 探测结果的字符串引用，将被填充为目标路径。
     * @return 返回操作的结果，0 表示成功，非零值表示失败。
     */
    [[nodiscard]] int run(string& result);
};

/**
 * @brief 将Option对象转换为Sniff_Hint对象
 *
 * @param option Option对象
 * @return Sniff_Hint 转换后的Sniff_Hint对象
 */
Sniff_Hint convert_option_to_sniff_hint(const Option& option);
