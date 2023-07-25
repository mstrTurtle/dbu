#include "Option.h"

#include "ace/Get_Opt.h"
#include <ace/OS_NS_string.h>

Option *Option::instance_ = 0;


Option *
Option::instance () // 单例模式实现
{
  if (Option::instance_ == 0)
    Option::instance_ = new Option;

  return Option::instance_;
}



void
Option::parse_args (int argc, ACE_TCHAR *argv[]) // 此函数给结构体赋上值，并且视情况打Log
{
  ACE_Get_Opt get_opt (argc, argv, ACE_TEXT ("b:s:u:a:p:t:d"));

  int c;

  while ((c = get_opt ()) != -1)
    switch (c)
      {
      case 'd':
        this->debug_ = 1;
        break;
      case 'b':
        this->branch_ = get_opt.opt_arg (); // 通过get_opt.opt_arg()获得值。
        break;
      case 's':
        this->sub_branch_ = get_opt.opt_arg ();
        break;
      case 'u':
        this->build_ = get_opt.opt_arg ();
        break;
      case 'a':
        this->arch_ = get_opt.opt_arg (); // 通过atoi做转换。
        break;
      case 'p':
        this->product_ = get_opt.opt_arg ();
        break;
      case 't':
        this->threads_ = ACE_OS::atoi (get_opt.opt_arg ());
        break;
      default: // DEBUG模式下失配时，需要输出LOG。
        ACE_DEBUG ((LM_DEBUG, "%s -h hostname -f filename -[r/w] [-p port] [-l length] [-o offset] [-d]\n", argv[0]));
        ACE_OS::exit (1);
      }

  if (this->branch_.length() == 0 || this->sub_branch_.length() == 0) // hostname和filename不可不输入
    {
      ACE_DEBUG ((LM_DEBUG,
                  "%s -b branch -s sub_branch -b build -a arch -p product -t threads [-d]\n",
                  argv[0]));

      ACE_OS::exit (1);
    }

}

Option::Option () // 构造器给结构体赋初值。
  : branch_ (0),
    sub_branch_ (0),
    build_ (0),
    arch_ (0),
    product_ (0),
    threads_ (1),
    debug_ (0)
{
}

struct PathBuilder{
    string& origin_;

    PathBuilder(string& origin):origin_(origin){}
    
    PathBuilder& add(const string& appendix){
        if((appendix.length()) == 0){
            return *this;
        }

        if (origin_.back() != '/'){
            origin_.push_back('/');
        }

        
        if(appendix[0] != '/'){
            origin_ += (appendix.substr(1));
        }
        else{
            origin_ += appendix;
        }

        return *this;
    }
};

string 
Option::get_actual_path(){
    string path = string("/ftp_product_installer/dbackup3/rpm");
    PathBuilder(path)
            .add(branch_)
            .add(sub_branch_)
            .add(build_)
            .add(arch_)
            .add(product_)
            ;
    return path;
}
