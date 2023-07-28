/**
 * @file Option.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 解析命令行选项的类
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include "ace/ARGV.h"
#include <string>

using std::string;

/**
 * @brief 选项解析器类
 * 
 */
class Option
{
public:
  /// Returns the singleton instance
  static Option* instance();

  void parse_args(int argc, ACE_TCHAR* argv[]);

  string branch_;
  string sub_branch_;
  string build_;
  string arch_;
  string product_;
  int threads_;
  bool debug_;

  string get_actual_path();

protected:
  Option(); // 放protected里，因为要确保单例。
  // protected constructor, singleton

  /// the singleton
  static Option* instance_;
};

#define OPTION (Option::instance())
