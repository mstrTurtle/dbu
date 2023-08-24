#pragma once
#include "Sniffer.h"
#include "cstring"
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <algorithm>
#include <string>
#include <vector>
#include <tuple>

#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

using std::string;
using std::vector;

using VS = vector<string>;

using SOCK = ACE_SOCK_Stream;

using Str = string;

void
fetchNLST(ACE_SOCK_Stream& control_socket,
          const std::string& cwd,
          std::string& result);

int
sniff(char* buffer, int size);

void
join_path(std::string& origin_, const std::string& appendix);

std::vector<std::string>
strToLines(string text);

VS
fzf(VS ss, string e);

int
findMax(const VS& ss, std::string& result);

int
fetchFindMax(SOCK sock, Str path, Str& result);

int
fetchFzf(SOCK sock, Str path, Str e, VS& result);

bool
fetchFind(SOCK sock, Str path, Str e);

bool
fetchExist(SOCK sock, Str path);

void
setupControl(ACE_SOCK_Stream& control_socket);

int getStatusCode(const char* line);

int getStatusCode(Str line);

int
getRegularName(string path, string& result);
