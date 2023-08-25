/**
 * @file downloader.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 输入路径，提供多线程下载功能、进度跟踪功能。
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "Option.h"
#include "ftp_operation.h"
#include <string>

#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

using std::string;

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

class Downloader
{
    string filepath_;
    int threads_;
    string savepath_;
    SOCK sock_;
    SockCreator sock_creator_;

public:
    Downloader(
            string path,
            int threads,
            string savepath,
            SOCK sock,
            SockCreator sock_creator)
        : filepath_(path),
          threads_(threads),
          savepath_(savepath),
          sock_(sock),
          sock_creator_(sock_creator)
    {
    }
    int run();
};
