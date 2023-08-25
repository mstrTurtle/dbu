#pragma once
#include "sniffer.h"
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

int fetch_nlst(
        ACE_SOCK_Stream& control_socket,
        const std::string& cwd,
        std::string& result);

int sniff(char* buffer, int size);

void join_path(std::string& origin_, const std::string& appendix);

std::vector<std::string> str_to_lines(string text);

VS fzf(VS ss, string e);

int find_max(const VS& ss, std::string& result);

int fetch_find_max(SOCK sock, Str path, Str& result);

int fetch_fzf(SOCK sock, Str path, Str e, VS& result);

bool fetch_find(SOCK sock, Str path, Str e);

bool fetch_exist(SOCK sock, Str path);

void setup_control(ACE_SOCK_Stream& control_socket);

int get_status_code(const char* line);

int get_status_code(Str line);

int get_regular_name(string path, string& result);
