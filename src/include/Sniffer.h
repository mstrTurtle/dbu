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

struct SniffHint {
  const string branch;
  const string subbranch;
  const string option;
  const string arch;
  const string product;

  friend std::ostream& operator<<(std::ostream& os, const SniffHint& hint) {
    os << "Branch: " << hint.branch << std::endl;
    os << "Subbranch: " << hint.subbranch << std::endl;
    os << "Option: " << hint.option << std::endl;
    os << "Arch: " << hint.arch << std::endl;
    os << "Product: " << hint.product << std::endl;
    return os;
  }
};

class Sniffer final {
private:
  struct {
    ACE_INET_Addr addr;
    SOCK sock;
  } conn;
  SniffHint hint;
  string cwd;

public:
  Sniffer() = delete;
  Sniffer(ACE_INET_Addr addr_, SOCK sock_, SniffHint hint_)
    : conn({ .addr = addr_, .sock = sock_ })
    , hint(hint_){};
  int processBranch();
  int processOption();
  int processTarget();
  int processVersion();
  int processFunctionality();
  int run(Str& result);
};

SniffHint
convertOptionToSniffHint(const Option& option);
