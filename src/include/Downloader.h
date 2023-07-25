#pragma once
#include <string>
#include "Option.h"

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
