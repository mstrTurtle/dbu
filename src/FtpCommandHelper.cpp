#include "FtpCommandHelper.h"

#include <string>

using std::string;

FtpCommandHelper::FtpCommandHelper(){};

string
FtpCommandHelper::loginPhase()
{
  return "USER scutech\nPASS dingjia\n";
};

string
FtpCommandHelper::retr(const string& path)
{
  return "RETR " + path + '\n';
};
