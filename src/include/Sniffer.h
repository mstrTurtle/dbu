#pragma once

#include "FtpOperation.h"
#include "FtpUtil.h"
#include "Option.h"
#include "Sniffer.h"
#include "cstring"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using std::vector;
using std::string;

using VS = vector<string>;

struct FtpConn {
  string ip;
  int port;
  SOCK sock;
};

struct SniffHint {
  const string branch;
  const string subbranch;
  const string option;
  const string arch;
  const string product;
};

class Sniffer final {
private:
  FtpConn conn;
  SniffHint hint;
  string cwd;

public:
  Sniffer() = delete;
  Sniffer(FtpConn conn_, SniffHint hint_)
    : conn(conn_)
    , hint(hint_){};
  int processBranch();
  int processOption();
  int processTarget();
  int processVersion();
  int processFunctionality();
  int run(Str& result);
};
