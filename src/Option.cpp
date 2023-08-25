#include "Option.h"

#include "ace/Get_Opt.h"
#include "ace/OS_NS_string.h"

#include <string>

using std::string;

void printUsage(const char* pname)
{
    ACE_DEBUG(
            (LM_DEBUG,
             "Usage:\n"
             "%s -h hostname -f filename -[r/w] [-p port] [-l length] "
             "[-o offset] [-d]\n",
             pname));
}

Option* Option::instance_ = nullptr;

Option* Option::instance() // 单例模式实现
{
    if (Option::instance_ == 0)
        Option::instance_ = new Option;

    return Option::instance_;
}

/**
 * @brief 解析命令行参数并给选项结构体赋值。
 *
 * @param argc 参数个数
 * @param argv 参数数组
 *
 * 此函数会解析命令行参数，并根据参数的不同情况进行相应的操作，并记录日志。
 */
void Option::parse_args(int argc, ACE_TCHAR* argv[])
{
    ACE_Get_Opt get_opt(argc, argv, ACE_TEXT("b:s:u:a:p:t:d"));

    int c;

    while ((c = get_opt()) != -1) {
        // ACE_DEBUG((LM_DEBUG, "got %c \n", c));
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

    if (this->branch_ == "") // hostname和filename不可不输入
    {
        printUsage(argv[0]);
        ACE_OS::exit(1);
    }
}

/**
 * @brief Option类的构造函数，用于给选项结构体赋初值。
 *
 * 构造函数会将选项结构体的各个成员变量初始化为默认值。
 */
Option::Option() // 构造器给结构体赋初值。
    : branch_(""),
      sub_branch_(""),
      build_(""),
      arch_(""),
      product_(""),
      threads_(1),
      debug_(0)
{
}

[[deprecated]] std::string Option::get_actual_path()
{
    string path = string("/ftp_product_installer/dbackup3/rpm");
    return path;
}

std::ostream& operator<<(std::ostream& os, const Option& option)
{
    os << "branch: " << option.branch_ << std::endl;
    os << "sub_branch: " << option.sub_branch_ << std::endl;
    os << "build: " << option.build_ << std::endl;
    os << "arch: " << option.arch_ << std::endl;
    os << "product: " << option.product_ << std::endl;
    os << "threads: " << option.threads_ << std::endl;
    os << "debug: " << option.debug_ << std::endl;

    return os;
}
