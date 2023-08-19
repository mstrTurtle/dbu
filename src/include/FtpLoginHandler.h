#include "ace/ACE.h"
#include "ace/SOCK_Connector.h"
#include "ace/Svc_Handler.h"
#include "ace/Reactor.h"
#include "ace/Connector.h"
#include <ace/Event_Handler.h>

class FTPHandler;

using FTP_Connector = ACE_Connector<FTPHandler, ACE_SOCK_CONNECTOR> ;

class FTPHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_MT_SYNCH>
{
public:
  int open(void * = 0) override
  {
    // 执行FTP登录操作，首先构造登录命令
    const char *user = "scutech";
    const char *pass = "dingjia";
    ACE_OS::sprintf(loginCmd_, "USER %s\r\n", user);
    ACE_OS::sprintf(passCmd_, "PASS %s\r\n", pass);

    // 发送登录命令
    if (peer().send_n(loginCmd_, ACE_OS::strlen(loginCmd_)) == -1)
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Failed to send USER command\n")), -1);
    }

    // 发送密码命令
    if (peer().send_n(passCmd_, ACE_OS::strlen(passCmd_)) == -1)
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Failed to send PASS command\n")), -1);
    }

    // 注册处理程序以继续处理连接
    ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK);

    return 0;
  }

  int handle_input(ACE_HANDLE = ACE_INVALID_HANDLE) override
  {
    char buffer[1024];
    ssize_t bytesRead = peer().recv(buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
      buffer[bytesRead] = '\0';
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Received response: %s\n"), buffer));
    }
    else if (bytesRead == 0)
    {
      ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) Connection closed by peer\n")));
      ACE_Reactor::instance()->remove_handler(this, ACE_Event_Handler::READ_MASK);
      delete this;
    }
    else
    {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Error in handle_input\n")), -1);
    }

    return 0;
  }

private:
  char loginCmd_[64];
  char passCmd_[64];
};

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  // 指定FTP服务器域名和端口号
  ACE_INET_Addr serverAddr("ftp.scutech.com", 21);

  // 创建Connector，实例化Handler，并发起连接
  FTP_Connector connector;
  FTPHandler *handler = new FTPHandler();
  if (connector.connect(handler, serverAddr) == -1)
  {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) Failed to connect to FTP server\n")), 1);
  }

  // 运行事件循环
  ACE_Reactor::instance()->run_reactor_event_loop();

  return 0;
}
