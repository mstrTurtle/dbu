#include "Sniffer.h"
#include "cstring"
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <algorithm>
#include <string>
#include <vector>

#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <iostream>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using std::vector;
using std::string;

using VS = vector<string>;

// forward declaration
void
fetchNLST(ACE_SOCK_Stream& control_socket,
          const std::string& cwd,
          const std::string& data_ip,
          u_short data_port,
          std::string& result);

/**
 * @brief 输入Option，输出嗅探到的文件路径
 *
 * @return int 错误码，0是对的，其他是其他错误码
 */
int
sniff(char* buffer, int size)
{
  if (size <= 200) {
    return 1;
  }
  if (strncpy_s(
        buffer,
        size,
        "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/debug/x86_64/"
        "8.0.35618/dbackup3-agent-8.0.35618-1.6586071.dbg.x86_64.rpm",
        200)) {
    return 2;
  }
  return 0;
}

void
join_path(std::string& origin_, const std::string& appendix)
{
  if ((appendix.length()) == 0) {
    return origin_;
  }

  if (origin_.back() != '/') {
    origin_.push_back('/');
  }

  if (appendix[0] == '/') {
    origin_ += (appendix.substr(1));
  }
  else {
    origin_ += appendix;
  }

  return;
}

std::vector<std::string>
strToLines(string text)
{
  std::vector<std::string> result;
  std::istringstream iss(text);
  std::string line;

  while (std::getline(iss, line, '\n')) {
    if (line.length() >= 1 && line.back() == '\r') {
      line.pop_back();
    }
    result.push_back(line);
  }
  return result;
}

/**
 * @brief 找是否出现过
 *
 * @param v
 * @param e
 * @return true
 * @return false
 */
bool
find(vector<string> v, std::string e)
{
  return std::find(v.begin(), v.end(), e) == v.end();
}

/**
 * @brief fuzzy find.
 *
 * @param ss
 * @param e
 * @return VS
 */
VS
fzf(VS ss, string e)
{
  VS result;
  for (auto s : ss) {
      if (s.find(e) != string::npos) {
        result.emplace_back(s);
      }
  }
  return result;
}

// 函数用于找最大版本号的辅助函数
int
findMax(const VS& ss, std::string& result)
{
  std::istringstream iss(text);
  std::string line;
  std::tuple<int, int, int> maxNumber(0, 0, 0);

  if (lines.length() < 1)
    return 1;

  for (auto line : lines) {
    // 将数字解析为tuple
    int a, b, c;
    char dot;
    std::istringstream numberStream(number);
    sscanf(line.c_str(), "%d.%d.%d", &a, &b, &c);
    std::tuple<int, int, int> currentNumber(a, b, c);
    if (maxNumber < currentNumber) {
      maxNumber = currentNumber;
      result = line;
    }
  }
  return 0;
}

/**
 * @brief 组合出findMax函数
 *
 * @return int
 */
int
fetchFindMax(){
  v = fetchNLST(int &control_socket, const std::string &cwd, const std::string &data_ip, int data_port, std::string &result);
  return findMax(ss, result);
}

/**
 * @brief 组合出fzf函数
 *
 * @return true
 * @return false
 */
bool
fetchFzf(){
  s = fetchNLST();
  return fzf(strToLines(s), e);
}

/**
 * @brief 组合出find函数
 *
 * @return VS
 */
VS
fetchFind(){
  s = fetchNLST();
  return find(strToLines(s), e);
}

bool
fetchExist(){
  s = fetchNLST(, , , , );
  return strToLines(s).length() == 1;
}


char buffer[1024];
ssize_t recv_count;

/**
 * FTP登录与转入被动
 */
void
setupControl(ACE_SOCK_Stream& control_socket,
             std::string& data_ip,
             u_short& data_port)
{
  // current working directory
  std::string cwd = "/ftp_product_installer/dbackup3/rpm";

  // 接收服务器的欢迎信息
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  // 发送用户名和密码
  control_socket.send("USER scutech\r\n", 16);
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  control_socket.send("PASS dingjia\r\n", 6);
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  // 发送PASV命令，进入被动模式
  control_socket.send("PASV\r\n", 6);
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  // 解析被动模式响应，获取数据连接IP和端口号
  unsigned int ip1, ip2, ip3, ip4, port1, port2;
  sscanf(buffer,
         "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
         &ip1,
         &ip2,
         &ip3,
         &ip4,
         &port1,
         &port2);
  data_ip = std::to_string(ip1) + "." + std::to_string(ip2) + "." +
            std::to_string(ip3) + "." + std::to_string(ip4);
  data_port = (port1 << 8) + port2;
}

/**
 * @brief 抓取目录列表
 *
 * @param control_socket
 * @param cwd
 * @param data_ip
 * @param data_port
 * @param result
 */
void
fetchNLST(ACE_SOCK_Stream& control_socket,
          const std::string& cwd,
          const std::string& data_ip,
          u_short data_port,
          std::string& result)
{
  // 建立数据连接
  ACE_SOCK_Stream data_socket;
  ACE_INET_Addr data_addr(data_port, data_ip.c_str());
  ACE_SOCK_Connector connector;
  if (connector.connect(data_socket, data_addr) == -1) {
    ACE_DEBUG((LM_ERROR, "Error connecting to data socket.\n"));
    return;
  }

  // 发送NLST命令
  ACE_OS::sprintf(buffer, "NLST %s\r\n", cwd.c_str());
  control_socket.send(buffer, ACE_OS::strlen(buffer));
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  // 接收数据
  std::string received_lines;
  while ((data_socket.recv(buffer, sizeof(buffer))) > 0) {
    received_lines += buffer;
  }

  // 关闭数据连接
  data_socket.close();

  // 接收NLST命令响应
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;

  // 结束流程并返回值
  result =  received_lines;
  return 0;
}

class ResourceSniffer{
  ACE_SOCK_Stream& control_socket;
  const std::string& cwd;
  u_short data_port;
  const std::string& data_ip;

  void enter(string path){
    join_path(this->cwd, path);
  }
};

// 函数用于判断主分支并处理分支
int
processBranch(const std::string& cwd,
              const std::string& branch,
              const std::string& subbranch)
{
  if (branch == "develop" || branch == "master") {
    join_path(cwd, subbranch);
    return 0;
  }
  else if (branch == "feature" || branch == "hotfix" || branch == "support") {
    bool b = fetchFind(control_socket, cwd, subbranch);
    if(!b){
      return 2;
    }
    join_path(cwd, subbranch);
    return 0;
  }
}

/**
 * @brief 处理选项
 *
 * @return int
 */
int
processOption(const std::string& cwd, ACE_SOCK_Stream& control_socket)
{
  fetchFind();
  join_path(cwd, option);
  return 0;
}

/**
 * @brief 处理目标
 *
 * @return int
 */
int
processTarget(cwd)
{
  if(fetchFind(cwd)){
    ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
    return 1;
  }
  join_path(cwd, option);
  return 0;
}

/**
 * @brief 处理版本号
 *
 * @return int
 */
int
processVersion(cwd)
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
processFunctionality(cwd)
{
  v = fetchFzf();
  join_path(cwd, option);
}

/**
 * 嗅探的入口。传入Option，逐步嗅探。返回错误码。
 * TODO: 我觉得得定义一下错误码。
 */
void
sniffRun(ACE_SOCK_Stream& control_socket, std::string& result, Option option)
{
  std::string data_ip;
  u_short data_port;

  std::string cwd = "/ftp_product_installer/dbackup3/rpm";


  if (ret = processBranch() | processOption() | processTarget() |
            processVersion() | processFunctionality()) {
    cout << "处理过程出错了，退出程序" << endl;
    exit(ret);
  }

  // 发送QUIT命令，关闭控制连接
  control_socket.send("QUIT\r\n", 6);
  recv_count = control_socket.recv(buffer, sizeof(buffer));
  buffer[recv_count] = '\0';
  std::cout << buffer;
}

int
run()
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
