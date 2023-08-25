#ifndef OPTION_H
#define OPTION_H

#include <ace/SOCK_Stream.h>
#include <string>

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

#include <iostream>
#include <string>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <functional>

typedef std::function<int(SOCK&)> SockCreator;

SockCreator make_logined_sock_creator(
        const ACE_INET_Addr& address,
        const std::string& username,
        const std::string& password);

#endif // OPTION_H
