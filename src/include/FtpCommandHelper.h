/**
 * @file FtpCommandHelper.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 处理FTP连接的Helper类
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "string"

using std::string;

class FtpCommandHelper
{
    FtpCommandHelper();
    string loginPhase();
    string retr(const string& path);
};
