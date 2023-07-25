#pragma once
#include "string"

using std::string;

class Downloader
{
  string filepath_;
  string threads_;
  string save_path_;
  Downloader(string option);
  int run();
};
