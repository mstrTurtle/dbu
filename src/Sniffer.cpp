#include "Sniffer.h"
#include "FtpOperation.h"
#include "cstring"
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <algorithm>
#include <string>
#include <vector>
#include <tuple>

#include "FtpUtil.h"
#include "FtpOperation.h"
#include "Option.h"

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


using std::vector;
using std::string;

using VS = vector<string>;

struct FtpConn
{
  string ip;
  int port;
  SOCK sock;
};

struct SniffHint
{
  string cwd;
  const string branch;
  const string subbranch;
  const string option;
  const string arch;
  const string product;
};

class Sniffer final
{
private:
  FtpConn conn;
  SniffHint hint;

public:
  Sniffer() = delete;
  int processBranch();
  int processOption();
  int processTarget();
  int processVersion();
  int processFunctionality();
};

// 函数用于判断主分支并处理分支
int
Sniffer::processBranch()
{
  if (hint.branch == "develop" || hint.branch == "master") {
    join_path(hint.cwd, hint.subbranch);
    return 0;
  }
  else if (hint.branch == "feature" || hint.branch == "hotfix" ||
           hint.branch == "support") {
    bool b = fetchFind(conn.sock, hint.cwd, hint.subbranch);
    if (!b) {
      return 2;
    }
    join_path(hint.cwd, hint.subbranch);
    return 0;
  }
  return 0;
}

/**
 * @brief 处理选项
 *
 * @return int
 */
int
Sniffer::processOption()
{
  fetchFind();
  join_path(hint.cwd, hint.option);
  return 0;
}

/**
 * @brief 处理目标
 *
 * @return int
 */
int
Sniffer::processTarget()
{
  if(fetchFind(hint.cwd)){
    ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
    return 1;
  }
  join_path(hint.cwd, hint.option);
  return 0;
}

/**
 * @brief 处理版本号
 *
 * @return int
 */
int
Sniffer::processVersion()
{
  fetchFindMax();
  enter();
}

/**
 * @brief 处理功能
 *
 * @return int
 */
int
Sniffer::processFunctionality()
{
  VS v;
  fetchFzf(conn.sock,hint.cwd,v);
  join_path(hint.cwd, hint.option);
}

/**
 * 嗅探的入口。传入Option，逐步嗅探。返回错误码。
 * TODO: 我觉得得定义一下错误码。
 */
int
Sniffer::run(Str& result)
{
  std::string data_ip;
  u_short data_port;

  std::string cwd = "/ftp_product_installer/dbackup3/rpm";

  if (int ret = processBranch() | processOption() | processTarget() |
            processVersion() | processFunctionality()) {
    std::cout << "处理过程出错了，退出程序" << std::endl;
    exit(ret);
  }

  if (quitAndClose(conn.sock)){
    return 1;
  }

  result = hint.cwd;
  return 0;

}

int
test_main()
{
  // 初始化ACE
  ACE::init();

  // 建立控制连接
  ACE_SOCK_Stream control_socket;
  ACE_INET_Addr control_addr(FTP_SERVER_PORT, FTP_SERVER_IP);
  ACE_SOCK_Connector connector;
  if (connector.connect(control_socket, control_addr) == -1) {
    ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
    return 1;
  }

  // 登录
  setupControl(control_socket, data_ip, data_port);

  sniffRun();


  // 关闭ACE
  ACE::fini();

  return 0;
}
