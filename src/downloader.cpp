#include "downloader.h"
#include "ftp_operation.h"
#include "ftp_util.h"
#include "option.h"
#include <cstdio>

#include "option.h"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <iostream>
#include <set>
#include <string>
#include <thread>

/**
 * @brief 将多个文件聚合到一个文件中。
 *
 * 该函数读取多个输入文件的内容，并将其写入一个输出文件中。
 *
 * @param inputFiles 存储输入文件的FILE*句柄的向量。
 * @param outputFile 表示输出文件的FILE*句柄。
 */
void aggregateFiles(const std::vector<FILE*>& inputFiles, FILE* outputFile)
{
    for (const auto& inputFile : inputFiles) {
        // 将输入文件的读取位置移动到文件末尾
        fseek(inputFile, 0, SEEK_END);
        long fileSize = ftell(inputFile);
        fseek(inputFile, 0, SEEK_SET);

        // 创建一个缓冲区来保存当前输入文件的内容
        std::vector<char> buffer(fileSize);

        // 从输入文件中读取内容到缓冲区
        fread(buffer.data(), sizeof(char), fileSize, inputFile);

        // 将缓冲区中的内容写入到输出文件
        fwrite(buffer.data(), sizeof(char), fileSize, outputFile);
    }
}

/**
 * @brief 打开多个文件
 *
 * 该函数用于打开多个文件，并将打开的文件指针存储在给定的vector中。
 *
 * @param n 要打开的文件数量。
 * @param result 存储打开的文件指针的vector。
 * @return 如果成功打开所有文件，则返回0；否则返回非零值。
 */
int open_n_file(int n, vector<FILE*>& result)
{
    char fname[100];
    for (int i = 0; i < n; i++) {
        sprintf(fname, "downloadfile.%d", i);
        FILE* file = ACE_OS::fopen(fname, "w+b");
        if (!file) {
            std::cout << "Open File Error\n";
            return 1;
        }
        result.emplace_back(file);
    }
    return 0;
}

/**
 * @brief 启动多个下载线程并等待它们完成，并将下载的文件片段合并到一个文件中。
 *
 * @param sock 下载的套接字。
 * @param path 下载的文件路径。
 * @param threads 启动的下载线程数。
 * @param savepath 合并后的文件保存路径。
 * @param create_sock 创建套接字的函数指针。
 * @return 返回0表示下载成功，返回1表示下载失败。
 */
int spawn_multi_downloads_and_join(
        SOCK sock,
        Str path,
        int threads,
        const char* savepath,
        Sock_Creator create_sock)
{
    std::cout << "spawning\n";
    std::vector<std::thread> ts; // 计算大小，并且spawn若干线程以供下载。
    std::vector<FILE*> fs;
    int fsize = get_ftp_file_size(sock, path);
    int fhandle = open(path.c_str(), O_RDWR);
    int segsize =
            static_cast<int>(static_cast<float>(fsize) / threads); // 向下取整

    std::cout << "got size " << fsize << "\n";

    if (open_n_file(threads, fs)) {
        std::cout << "open failed\n";
        return 1;
    }

    for (int i = 0; i < threads - 1; i++) {
        int off = i * segsize;
        int len = segsize;
        SOCK newSock;
        create_sock(newSock);
        ts.emplace_back(std::thread(
                enter_passive_and_download_one_segment_and_close, path, off, len, i,
                fs[i], newSock));

        std::cout << "Thread " << i << " Start" << std::endl;
    }

    int finaloff = (threads - 1) * segsize;
    SOCK newSock;
    create_sock(newSock);
    ts.emplace_back(std::thread(
            enter_passive_and_download_one_segment_and_close, path, finaloff,
            fsize - finaloff, threads - 1, fs.back(), newSock));
    std::cout << "Thread " << (threads - 1) << " Start" << std::endl;

    std::cout << "Download Complete" << std::endl;

    for (auto& t : ts) {
        t.join();
    }

    FILE* file = ACE_OS::fopen(savepath, "wb");
    aggregateFiles(fs, file);

    for (auto& f : fs) {
        ACE_OS::fclose(f);
    }

    ACE_OS::fclose(file);

    return 0;
}

/**
 * @brief 下载器运行函数
 *
 * 该函数是下载器的主要运行函数。它通过初始化ACE库，建立与服务器的连接并登录，然后调用
 * spawn_multi_downloads_and_join 函数进行多线程下载和合并操作，最后关闭ACE库。
 *
 * @return 如果运行成功，则返回0；否则返回非零值。
 */
int Downloader::run()
{
    // 初始化ACE
    ACE::init();

    // 处理控制连接void
    spawn_multi_downloads_and_join(
            sock_, filepath_, threads_, savepath_.c_str(), sock_creator_);

    // 关闭ACE
    ACE::fini();

    return 0;
}
