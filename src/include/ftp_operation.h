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

SOCK connect_to_ftp(string ip, int port = 21);
int login_to_ftp(
        SOCK sock,
        string user = "anonymous",
        string pass = "");
int enter_passive_and_get_data_connection(SOCK sock, SOCK& dsock);
void download_one_segment(
        SOCK sock,
        SOCK data_socket,
        string path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file);
int quit_and_close(ACE_SOCK_Stream& sock);
int get_ftp_file_size(SOCK sock, const std::string& path);
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
 */
typedef std::function<int(SOCK&)> Sock_Creator;

/**
 * @brief 新建闭包，产生FTP控制连接工厂方法
 *
 * 本函数返回一个创建一个连接到FTP服务器的新socket的闭包，
 * 生成的socket将用指定的用户名和密码登录。
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
    int sendCommand(const std::string& command, const std::string& argument);
    /**
     * @brief 接收FTP服务器的回复。
     * @param status_code 存储接收到的状态码
     * @param line 存储接收到的行内容
     * @return 接收成功返回0，接收失败或连接关闭返回1
     */
    int receiveReply(std::string& status_code, std::string& result_lines);
};
