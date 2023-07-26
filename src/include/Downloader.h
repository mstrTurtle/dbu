// -*- C++ -*-

//=============================================================================
/**
 *  @file    Downloader.h
 *  @details 输入路径，提供多线程下载功能、进度跟踪功能。
 *
 *  @author JiayuXiao <xiaojiayu@scutech.com>
 */
//=============================================================================
#pragma once
#include "Option.h"
#include <string>

using std::string;

class Downloader
{
  string filepath_;
  string threads_;
  string save_path_;

public:
  Downloader();
  int run(Option& option);
};
