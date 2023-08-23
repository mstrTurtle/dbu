/**
 * @file Downloader.h
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
#include <string>

using std::string;

class Downloader
{
  string filepath_;
  int threads_;
  string save_path_;

public:
  Downloader(string path, int threads):filepath_(path), threads_(threads){}
  int run();
};
