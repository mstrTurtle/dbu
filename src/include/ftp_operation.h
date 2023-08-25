/**
 * @file ftp_operation.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 与FTP协议直接相关的操作，对FTP协议的高级抽象
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "ftp_util.h"
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <atomic>
#include <functional>
#include <iostream>
#include <string>

/**
 * @class Ftp_Control_Client
 * @brief 用于发送和接收FTP命令的FTP控制客户端类。
 */
class Ftp_Control_Client
{
private:
    Lined_SOCK sock; ///< Lined_SOCK对象

public:
    /**
     * @brief 构造函数，使用给定的SOCK对象初始化Ftp_Control_Client。
     * @param sock_ Lined_SOCK对象
     */
    Ftp_Control_Client(const SOCK& sock_): sock(sock_) {}

    /**
     * @brief 发送FTP命令。
     *
     * 发送一行FTP命令到服务器
     *
     * @param command 命令名
     * @param argument 命令内容
     * @return 发送成功返回0，发送失败返回1
     */
    [[nodiscard]] int send_command(
            const std::string& command,
            const std::string& argument);

    /**
     * @brief 接收FTP服务器的回复。
     *
     * 接收FTP服务器的回复，直到遇到代表结束的状态码
     *
     * @param status_code 存储接收到的状态码
     * @param line 存储接收到的行内容
     * @return 接收成功返回0，接收失败或连接关闭返回1
     */
    [[nodiscard]] int receive_reply(
            std::string& status_code,
            std::string& result_lines);

    /**
     * @brief 发送FTP命令并接收FTP服务器的回复。
     * @param command 命令名
     * @param argument 命令内容
     * @param status_code [out] 存储接收到的状态码
     * @param line [out] 存储接收到的行内容
     * @return 发送与接收都成功返回0，发送或接收失败或连接关闭返回1
     */
    [[nodiscard]] int send_and_receive(
            const std::string& command,
            const std::string& argument,
            std::string& status_code,
            std::string& result_lines);
};

/**
 * @brief 登录到FTP服务器。
 *
 * 此函数向FTP服务器发送用户名和密码以进行登录。
 *
 * @param cli 控制连接的套接字。
 * @param user FTP服务器的用户名。
 * @param pass FTP服务器的密码。
 * @return 如果成功登录，则返回0；如果出现错误，则返回1。
 */
[[nodiscard]] int login_to_ftp(
        Ftp_Control_Client cli,
        string user = "anonymous",
        string pass = "");

/**
 * @brief 进入被动模式并建立数据连接。
 *
 * 此函数向FTP服务器发送PASV命令以进入被动模式，并解析响应以获取数据连接的IP地址和端口号。
 * 然后，它使用获取的IP地址和端口号建立数据连接。
 *
 * @param cli Ftp_Control_Client 对象，用于发送控制命令和接收响应。
 * @param dsock 数据连接的套接字。
 * @return 如果成功建立数据连接，则返回0；如果出现错误，则返回1。
 */
[[nodiscard]] int enter_passive_and_get_data_connection(
        Ftp_Control_Client cli,
        SOCK& dsock);

/**
 * @brief 从FTP服务器下载文件的一部分。
 *
 * 此函数使用提供的控制套接字和数据套接字从FTP服务器下载文件的一部分。
 *
 * @param cli Ftp_Control_Client 对象，用于发送控制命令和接收响应
 * @param data_socket 用于接收文件数据的数据套接字。
 * @param path FTP服务器上文件的路径。
 * @param start_offset 要下载的部分的起始偏移量。
 * @param size 要下载的部分的大小。
 * @param part_id 正在下载的部分的ID。
 * @return 如果成功，则返回0；如果出现错误，则返回1。
 *
 * @note 此函数假设控制套接字和数据套接字已经连接到FTP服务器。
 */
[[nodiscard]] int download_one_segment(
        Ftp_Control_Client cli,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file,
        std::atomic<bool>& canceled);

/**
 * @brief 关闭控制连接并退出FTP服务器。
 *
 * 此函数发送QUIT命令关闭控制连接，并从FTP服务器退出。
 *
 * @param sock 控制连接的套接字。
 * @return 返回值为0表示成功关闭控制连接和退出FTP服务器，否则表示出现错误。
 */
[[nodiscard]] int quit_and_close(SOCK& sock);

/**
 * @brief 获取FTP服务器上文件的大小。
 *
 * 此函数向FTP服务器发送SIZE命令以获取指定文件的大小。
 *
 * @param cli Ftp_Control_Client 对象，用于发送控制命令和接收响应。
 * @param path 文件的路径。
 * @param result [out] 查找结果，文件的大小
 * @return 返回操作的结果，0 表示成功，非零值表示失败。
 */
[[nodiscard]] int get_ftp_file_size(
        Ftp_Control_Client cli,
        const std::string& path,
        int& result);

/**
 * @brief FTP控制连接socket的工厂方法类型
 *
 * Sock_Creator 定义了一个函数类型，它是一个
 * 闭包。函数调用后返回一个连接到目标服务器并且完成
 * 特定操作的socket。
 */
typedef std::function<int(SOCK&)> Sock_Creator;

/**
 * @brief 新建闭包，产生FTP控制连接工厂方法
 *
 * 本函数返回一个闭包，该闭包创建新的ACE_SOCK_Stream。
 * 返回时的ACE_SOCK_Stream已经连接到目标FTP服务器，并且已经以
 * 闭包中存储的用户名和密码登录。
 *
 * @param address 远端地址
 * @param username 用户名
 * @param password 密码
 * @return Sock_Creator
 */
[[nodiscard]] Sock_Creator make_logined_sock_creator(
        const ACE_INET_Addr& address,
        const std::string& username,
        const std::string& password);

/**
 * @brief 抓取目录列表
 *
 * 通过与服务器建立数据连接，发送NLST命令来获取指定目录下的文件列表，
 * 并将结果存储在result参数中。
 *
 * @param cli Ftp_Control_Client 对象，用于发送控制命令和接收响应
 * @param cwd 目标目录路径
 * @param result 存储目录列表的字符串引用，将被填充为获取到的文件列表
 * @return 返回操作的结果，0 表示成功，非零值表示失败。
 */
[[nodiscard]] int fetch_nlst(
        Ftp_Control_Client cli,
        const std::string& cwd,
        std::string& result);

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
[[nodiscard]] int fetch_find_max(SOCK sock, string path, string& result);

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
[[nodiscard]] int fetch_fzf(SOCK sock, string path, string e, VS& result);

/**
 * @brief 从指定的 SOCK 连接中获取文件列表，并查找指定的文件路径。
 *
 * @param sock 连接的 SOCK 对象。
 * @param path 文件路径。
 * @param e 文件名。
 * @param result [out] 查找结果，true 表示找到，false 表示未找到。
 * @return 返回操作的结果，0 表示成功，非零值表示失败。
 */
[[nodiscard]] int fetch_find(SOCK sock, string path, string e, bool& result);

/**
 * @brief 下载线程入口：进入被动模式、下载一个分段并关闭连接
 *
 * 这个函数用于进入被动模式，下载一个指定的文件分段，并关闭连接。
 * 由于std::thread无法取得返回值，所以返回类型为void。
 *
 * @param path 文件路径
 * @param off 分段的偏移量
 * @param size 分段的大小
 * @param part_id 分段的ID
 * @param file 文件指针，用于写入下载的数据
 * @param sock 控制连接的 SOCK 对象
 * @param canceled 原子标志，用于取消下载
 */
void enter_passive_and_download_one_segment_and_close(
        string path,
        off_t off,
        size_t size,
        int part_id,
        FILE* file,
        SOCK sock,
        std::atomic<bool>& canceled);

/**
 * @brief 下载一个文件分段
 *
 * 这个函数用于从服务器下载一个文件分段。它接收一个控制连接对象、数据连接套接字、文件路径、起始偏移量、分段大小、分段ID、文件指针以及取消标志作为参数。
 *
 * @param cli Ftp_Control_Client 对象，用于发送控制命令和接收响应
 * @param data_socket 数据连接套接字
 * @param path 文件路径
 * @param start_offset 分段的起始偏移量
 * @param size 分段的大小
 * @param part_id 分段的ID
 * @param file 文件指针，用于写入下载的数据
 * @param canceled 原子标志，用于取消下载
 * @return 如果下载成功，则返回0；如果下载过程中或出错时取消了下载，则返回1
 */
[[nodiscard]] int download_one_segment(
        Ftp_Control_Client cli,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file,
        std::atomic<bool>& canceled);
