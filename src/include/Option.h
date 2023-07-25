#pragma once
#include "ace/ARGV.h"
#include "string"

using std::string;

class Option
{
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
