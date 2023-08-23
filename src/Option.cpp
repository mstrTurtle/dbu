#include "Option.h"

#include "ace/Get_Opt.h"
#include "ace/OS_NS_string.h"

#include <string>

using std::string;

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

  while ((c = get_opt()) != -1)
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
        ACE_DEBUG((LM_DEBUG,
                   "%s -h hostname -f filename -[r/w] [-p port] [-l length] "
                   "[-o offset] [-d]\n",
                   argv[0]));
        ACE_OS::exit(1);
    }

  if (this->branch_.length() == 0 ||
      this->sub_branch_.length() == 0) // hostname和filename不可不输入
  {
    ACE_DEBUG((LM_DEBUG,
               "%s -b branch -s sub_branch -b build -a arch -p product -t "
               "threads [-d]\n",
               argv[0]));

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

/**
 * @struct PathBuilder
 * @brief PathBuilder结构体用于构建路径字符串
 */
struct PathBuilder {
  /**
   * @brief 初始路径字符串
   */
  string& origin_;

  /**
   * @brief 构造函数
   * @param origin 初始路径字符串
   */
  PathBuilder(string& origin)
    : origin_(origin)
  {
  }

  /**
   * @brief 将附加字符串追加到路径字符串中
   * @param appendix 附加字符串
   * @return 修改后的PathBuilder对象的引用
   */
  PathBuilder& add(const string& appendix)
  {
    if ((appendix.length()) == 0) {
      return *this;
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

    return *this;
  }
};

std::string
Option::get_actual_path()
{
  string path = string("/ftp_product_installer/dbackup3/rpm");
  if(sub_branch_!="")
    PathBuilder(path).add(branch_).add(sub_branch_).add(build_).add(arch_);
  else
    PathBuilder(path).add(branch_).add(build_).add(arch_);
  // TODO：在此放一个检测的。需要去检测最新的包。
  return path;
}
