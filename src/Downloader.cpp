#include "Downloader.h"
#include "FtpOperation.h"
#include "FtpUtil.h"
#include "Option.h"
#include <cstdio>

#include "Option.h"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>
#include <set>
#include <string>
#include <thread>

using Str = std::string;
using SOCK = ACE_SOCK_Stream;

/**
 * @brief Spawns multiple threads to download a file in segments and joins them.
 *
 * This function divides the file into segments and creates multiple threads to
 * download each segment concurrently. After all the threads have completed
 * their tasks, this function waits for them to join before returning.
 *
 * @note This function assumes that the necessary variables and functions (e.g.,
 * `getSize`, `open`) are defined and initialized properly.
 *
 * @return void
 */
void
spawnMultiDownloadsAndJoin(SOCK sock, Str path, int threads)
{
  std::cout << "spawning\n";
  std::vector<std::thread> ts; // 计算大小，并且spawn若干线程以供下载。
  int fsize = getFtpFileSize(sock, path);
  int fhandle = open(path.c_str(), O_RDWR);
  int segsize =
    static_cast<int>(static_cast<float>(fsize) / threads); // 向下取整

  std::cout << "got size "<< fsize <<"\n";

  for (int i = 0; i < threads - 1; i++) {
    int off = i * segsize;
    int len = segsize;

    ts.emplace_back(std::thread(connectLoginAndDownloadOneSegmentFromVim, path, off, len,i));

    std::cout << "Thread " << i << " Start" << std::endl;
  }

  int finaloff = (threads - 1) * segsize;
  ts.emplace_back(std::thread(connectLoginAndDownloadOneSegmentFromVim, path, finaloff, fsize - finaloff,threads-1));
  std::cout << "Thread " << (threads - 1) << " Start" << std::endl;

  std::cout << "Download Complete" << std::endl;

  for (auto& t : ts) {
    t.join();
  }
}

int
Downloader::run()
{
  // 初始化ACE
  ACE::init();

  SOCK sock = connectAndLoginVimFtp();

  // 处理控制连接void
  spawnMultiDownloadsAndJoin(sock, filepath_, threads_);

  // 关闭ACE
  ACE::fini();

  return 0;
}
