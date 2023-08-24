#include "Option.h"

#include "ace/Get_Opt.h"
#include "ace/OS_NS_string.h"

#include <string>

using std::string;

void
printUsage(const char* pname)
{
  ACE_DEBUG((LM_DEBUG,
             "Usage:\n"
             "%s -h hostname -f filename -[r/w] [-p port] [-l length] "
             "[-o offset] [-d]\n",
             pname));
}

Option* Option::instance_ = nullptr;

Option*
Option::instance() // 单例模式实现
{
  if (Option::instance_ == 0)
    Option::instance_ = new Option;

  return Option::instance_;
}

void
Option::parse_args(int argc,
                   ACE_TCHAR* argv[]) // 此函数给结构体赋上值，并且视情况打Log
{
  ACE_Get_Opt get_opt(argc, argv, ACE_TEXT("b:s:u:a:p:t:d"));

  int c;

  while ((c = get_opt()) != -1) {
    ACE_DEBUG((LM_DEBUG, "got %c \n", c));
    switch (c) {
      case 'd':
        this->debug_ = 1;
        break;
      case 'b':
        this->branch_ = get_opt.opt_arg(); // 通过get_opt.opt_arg()获得值。
        break;
      case 's':
        this->sub_branch_ = get_opt.opt_arg();
        break;
      case 'u':
        this->build_ = get_opt.opt_arg();
        break;
      case 'a':
        this->arch_ = get_opt.opt_arg(); // 通过atoi做转换。
        break;
      case 'p':
        this->product_ = get_opt.opt_arg();
        break;
      case 't':
        this->threads_ = ACE_OS::atoi(get_opt.opt_arg());
        break;
      default: // DEBUG模式下失配时，需要输出LOG。
        ACE_DEBUG((LM_DEBUG, "wrong option %c\n", c));
        printUsage(argv[0]);
        ACE_OS::exit(1);
    }
  }

    if (this->branch_=="") // hostname和filename不可不输入
    {
      printUsage(argv[0]);
      ACE_OS::exit(1);
    }
}

Option::Option() // 构造器给结构体赋初值。
  : branch_("")
  , sub_branch_("")
  , build_("")
  , arch_("")
  , product_("")
  , threads_(1)
  , debug_(0)
{
}

std::string
Option::get_actual_path()
{
  string path = string("/ftp_product_installer/dbackup3/rpm");
  return path;
}
