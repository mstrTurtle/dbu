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
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <string>
#include <iostream>
#include <string>
#include <functional>

#include "ftp_util.h"

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
     * @param command 命令名
     * @param argument 命令内容
     * @return 发送成功返回0，发送失败返回1
     */
    int send_command(const std::string& command, const std::string& argument);

    /**
     * @brief 接收FTP服务器的回复。
     * @param status_code 存储接收到的状态码
     * @param line 存储接收到的行内容
     * @return 接收成功返回0，接收失败或连接关闭返回1
     */
    int receive_reply(std::string& status_code, std::string& result_lines);

    /**
     * @brief 发送FTP命令并接收FTP服务器的回复。
     * @param command 命令名
     * @param argument 命令内容
     * @param status_code 存储接收到的状态码
     * @param line 存储接收到的行内容
     * @return 发送与接收都成功返回0，发送或接收失败或连接关闭返回1
     */
    int send_and_receive(
            const std::string& command,
            const std::string& argument,
            std::string& status_code,
            std::string& result_lines);
};

/**
 * @brief 建立与FTP服务器的控制连接。
 *
 * 此函数使用给定的IP地址和端口号建立与FTP服务器的控制连接。
 *
 * @param ip FTP服务器的IP地址。
 * @param port FTP服务器的端口号。
 * @return
 * 如果成功建立控制连接，则返回控制连接的套接字；如果出现错误，则返回1。
 */
SOCK connect_to_ftp(string ip, int port = 21);

/**
 * @brief 登录到FTP服务器。
 *
 * 此函数向FTP服务器发送用户名和密码以进行登录。
 *
 * @param sock 控制连接的套接字。
 * @param user FTP服务器的用户名。
 * @param pass FTP服务器的密码。
 * @return 如果成功登录，则返回0；如果出现错误，则返回1。
 */
int login_to_ftp(
        Ftp_Control_Client sock,
        string user = "anonymous",
        string pass = "");

/**
 * @brief 进入被动模式并建立数据连接。
 *
 * 此函数向FTP服务器发送PASV命令以进入被动模式，并解析响应以获取数据连接的IP地址和端口号。
 * 然后，它使用获取的IP地址和端口号建立数据连接。
 *
 * @param sock 控制连接的套接字。
 * @param dsock 数据连接的套接字。
 * @return 如果成功建立数据连接，则返回0；如果出现错误，则返回1。
 */
int enter_passive_and_get_data_connection(
        Ftp_Control_Client sock,
        SOCK& dsock);

/**
 * @brief 从FTP服务器下载文件的一部分。
 *
 * 此函数使用提供的控制套接字和数据套接字从FTP服务器下载文件的一部分。
 *
 * @param sock 用于向FTP服务器发送命令的控制套接字。
 * @param data_socket 用于接收文件数据的数据套接字。
 * @param path FTP服务器上文件的路径。
 * @param start_offset 要下载的部分的起始偏移量。
 * @param size 要下载的部分的大小。
 * @param part_id 正在下载的部分的ID。
 *
 * @note 此函数假设控制套接字和数据套接字已经连接到FTP服务器。
 */
void download_one_segment(
        Ftp_Control_Client cli,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file);

/**
 * @brief 关闭控制连接并退出FTP服务器。
 *
 * 此函数发送QUIT命令关闭控制连接，并从FTP服务器退出。
 *
 * @param sock 控制连接的套接字。
 * @return 返回值为0表示成功关闭控制连接和退出FTP服务器，否则表示出现错误。
 */
int quit_and_close(SOCK& sock);

/**
 * @brief 获取FTP服务器上文件的大小。
 *
 * 此函数向FTP服务器发送SIZE命令以获取指定文件的大小。
 *
 * @param sock 控制连接的套接字。
 * @param path 文件的路径。
 * @return 返回文件的大小（以字节为单位），如果获取失败则返回-1。
 */
int get_ftp_file_size(Ftp_Control_Client sock, const std::string& path);

/**
 * @brief 进入被动模式，下载指定片段的文件，并关闭连接。
 *
 * @param path 文件路径。
 * @param off 文件偏移量。
 * @param size 文件大小。
 * @param part_id 片段 ID。
 * @param file 文件指针。
 * @param sock 连接的 SOCK 对象。
 */
void enter_passive_and_download_one_segment_and_close(
        string path,
        off_t off,
        size_t size,
        int part_id,
        FILE* file,
        SOCK sock);

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
Sock_Creator make_logined_sock_creator(
        const ACE_INET_Addr& address,
        const std::string& username,
        const std::string& password);
