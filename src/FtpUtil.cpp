#include "FtpUtil.h"
#include "FtpOperation.h"

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
  if (strncpy(
        buffer,
        "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/debug/x86_64/"
        "8.0.35618/dbackup3-agent-8.0.35618-1.6586071.dbg.x86_64.rpm",
        size)) {
    return 2;
  }
  return 0;
}

void
join_path(std::string& origin_, const std::string& appendix)
{
  if ((appendix.length()) == 0) {
    return;
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
  std::string line;
  std::tuple<int, int, int> maxNumber{0, 0, 0};
  if (ss.size() < 1)
    return 1;

  for (auto line : ss) {
    // 将数字解析为tuple
    int a, b, c;
    char dot;
    sscanf(line.c_str(), "%d.%d.%d", &a, &b, &c);
    std::tuple<int, int, int> currentNumber{a, b, c};
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
fetchFindMax(SOCK sock, Str path, Str& result){
  Str s;
  fetchNLST(sock,path, s);
  findMax(strToLines(s), result);
  return 0;
}

/**
 * @brief 组合出fzf函数
 *
 * @return true
 * @return false
 */
VS
fetchFzf(SOCK sock, Str path, Str e)
{
  Str s;
  fetchNLST(sock, path, s);
  return fzf(strToLines(s), e);
}

/**
 * @brief 组合出find函数
 *
 * @return bool
 */
bool
fetchFind(SOCK sock, Str path, Str e)
{
  Str s;
  fetchNLST(sock, path, s);
  return find(strToLines(s), e);
}

bool
fetchExist(SOCK sock, Str path)
{
  Str s;
  fetchNLST(sock, path, s);
  return strToLines(s).size() == 1;
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
          std::string& result)
{
  SOCK data_socket;
  enterPassiveAndGetDataConnection(control_socket, data_socket);

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
  return;
}
