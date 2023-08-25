#include "downloader.h"
#include "downloader_errors.h"
#include "ftp_operation.h"
#include "ftp_util.h"
#include "option.h"
#include <cstdio>

#include "option.h"
#include <ace/INET_Addr.h>
#include <iostream>
#include <set>
#include <string>
#include <thread>

using namespace Downloader_Errors;

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
            return BAD_FILE_HANDLE;
        }
        result.emplace_back(file);
    }
    return OK;
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
        string path,
        int threads,
        const char* savepath,
        Sock_Creator create_sock)
{
    ACE_TRACE(ACE_TEXT(__func__));
    std::vector<std::thread> ts; // 计算大小，并且spawn若干线程以供下载。
    std::vector<FILE*> fs;
    std::atomic<bool> canceled(false);
    int fsize;
    if (get_ftp_file_size(sock, path, fsize)) {
        std::cout << "Get file size failed\n";
        return BAD_PATH;
    }
    int fhandle = open(path.c_str(), O_RDWR);
    // 分段大小向下取整
    int segsize = static_cast<int>(static_cast<float>(fsize) / threads);

    std::cout << "Got size: " << fsize << std::endl;

    if (open_n_file(threads, fs)) {
        std::cout << "Open n file handles failed\n";
        return BAD_FILE_HANDLE;
    }

    for (int i = 0; i < threads - 1; i++) {
        int off = i * segsize;
        int len = segsize;
        SOCK new_sock;
        create_sock(new_sock);
        ts.emplace_back(std::thread(
                enter_passive_and_download_one_segment_and_close, path, off,
                len, i, fs[i], new_sock, std::ref(canceled)));

        std::cout << "Download thread " << i << " start" << std::endl;
    }

    int finaloff = (threads - 1) * segsize;
    SOCK new_sock;
    create_sock(new_sock);
    ts.emplace_back(std::thread(
            enter_passive_and_download_one_segment_and_close, path, finaloff,
            fsize - finaloff, threads - 1, fs.back(), new_sock,
            std::ref(canceled)));
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

    return OK;
}

int Downloader::run()
{
    // 发出多个下载线程，并等待线程完成
    int ret = spawn_multi_downloads_and_join(
            sock_, filepath_, threads_, savepath_.c_str(), sock_creator_);
    if (ret) {
        return ret;
    }

    return OK;
}
