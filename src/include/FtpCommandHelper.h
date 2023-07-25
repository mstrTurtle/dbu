#pragma once
#include "string"

using std::string;

class FtpCommandHelper{
  FtpCommandHelper();
  string loginPhase();
  string retr(const string& path);
} ;
