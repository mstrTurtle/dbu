#include "Sniffer.h"
#include "FtpOperation.h"
#include "FtpUtil.h"
#include "Option.h"
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


// 函数用于判断主分支并处理分支
int
Sniffer::processBranch()
{
  if (hint.branch == "develop" || hint.branch == "master") {
    join_path(cwd, hint.subbranch);
    return 0;
  }
  else if (hint.branch == "feature" || hint.branch == "hotfix" ||
           hint.branch == "support") {
    bool b = fetchFind(conn.sock, cwd, hint.subbranch);
    if (!b) {
      return 2;
    }
    join_path(cwd, hint.subbranch);
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
  fetchFind(conn.sock, cwd, hint.option);
  join_path(cwd, hint.option);
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
  if(!fetchFind(conn.sock, cwd, hint.arch)){
    ACE_DEBUG((LM_ERROR, "你提供的arch信息是错的.\n"));
    return 1;
  }
  join_path(cwd, hint.arch);
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
  Str result;
  fetchFindMax(conn.sock, result);
  join_path(cwd, result);
  return 0;
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
  int err = fetchFzf(conn.sock,cwd,hint.product, v);
  join_path(cwd, hint.option);
  return 0;
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

  if (int err = processBranch() | processOption() | processTarget() |
            processVersion() | processFunctionality()) {
    std::cout << "处理过程出错了，退出程序" << std::endl;
    exit(err);
  }

  if (quitAndClose(conn.sock)){
    return 1;
  }

  result = cwd;
  return 0;

}

int
test_main()
{
  // 初始化ACE
  ACE::init();

  // 建立控制连接
  ACE_SOCK_Stream control_socket;
  ACE_INET_Addr control_addr("ftp.vim.org", "21");
  ACE_SOCK_Connector connector;
  if (connector.connect(control_socket, control_addr) == -1) {
    ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
    return 1;
  }

  // 登录
  setupControl(control_socket);

  Sniffer sniffer(control_addr,control_socket,{});
  Str result;
  sniffer.run(result);


  // 关闭ACE
  ACE::fini();

  return 0;
}
