/**
 * @file downloader.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 输入路径，提供多线程下载功能。
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "option.h"
#include "ftp_operation.h"
#include <string>

#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

using std::string;

class Downloader
{
    string filepath_;
    int threads_;
    string savepath_;
    SOCK sock_;
    Sock_Creator sock_creator_;

public:
    Downloader(
            string path,
            int threads,
            string savepath,
            SOCK sock,
            Sock_Creator sock_creator)
        : filepath_(path),
          threads_(threads),
          savepath_(savepath),
          sock_(sock),
          sock_creator_(sock_creator)
    {
    }
    int run();
};
