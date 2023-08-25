#pragma once
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <string>
#include <iostream>
#include <string>
#include <functional>

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

SOCK connect_to_ftp(Str ip, int port = 21);
int login_to_ftp(SOCK control_socket, Str user = "anonymous", Str pass = "");
int enter_passive_and_get_data_connection(SOCK control_socket, SOCK& dsock);
void download_one_segment(
        SOCK control_socket,
        SOCK data_socket,
        Str path,
        off_t start_offset,
        size_t size,
        int part_id,
        FILE* file);
int quit_and_close(ACE_SOCK_Stream& control_socket);
int get_ftp_file_size(SOCK sock, const std::string& path);
void enter_passive_and_download_one_segment_and_close(
        Str path,
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
