/**
 * @file downloader.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 输入路径，提供多线程下载功能
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "ftp_operation.h"
#include "option.h"
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <string>

using std::string;

/**
 * @brief Downloader 类用于下载文件
 */
class Downloader
{
    string filepath_;           /**< 文件路径 */
    int threads_;               /**< 线程数 */
    string savepath_;           /**< 保存路径 */
    SOCK sock_;                 /**< socket 信息 */
    Sock_Creator sock_creator_; /**< socket 创建器 */

public:
    /**
     * @brief Downloader 类的构造函数
     *
     * @param path 文件路径
     * @param threads 线程数
     * @param savepath 保存路径
     * @param sock socket 信息
     * @param sock_creator socket 创建器
     */
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

    /**
     * @brief 下载器运行函数
     *
     * 该函数是下载器的主要运行函数。它通过初始化ACE库，建立与服务器的连接并登录，然后调用
     * spawn_multi_downloads_and_join
     * 函数进行多线程下载和合并操作，最后关闭ACE库。
     *
     * @return 如果运行成功，则返回0；否则返回非零值。
     */
    [[nodiscard]] int run();
};
