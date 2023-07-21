#include "ace/Get_Opt.h"
#include "ace/ARGV.h"
#include "Blob.h"
#include "Blob_Handler.h"
#include "Options.h"

Options *Options::instance_ = 0;

Options *
Options::instance () // 单例模式实现
{
  if (Options::instance_ == 0)
    Options::instance_ = new Options;

  return Options::instance_;
}

void
Options::parse_args (int argc, ACE_TCHAR *argv[]) // 此函数给结构体赋上值，并且视情况打Log
{
  ACE_Get_Opt get_opt (argc, argv, ACE_TEXT ("rwh:p:f:l:o:d"));

  int c;

  while ((c = get_opt ()) != -1)
    switch (c)
      {
      case 'd':
        this->debug_ = 1;
        break;
      case 'r':
        this->operation_ = 'r';
        break;
      case 'w':
        this->operation_ = 'w';
        break;
      case 'h':
        this->hostname_ = get_opt.opt_arg (); // 通过get_opt.opt_arg()获得值。
        break;
      case 'p':
        this->port_ = ACE_OS::atoi (get_opt.opt_arg ()); // 通过atoi做转换。
        break;
      case 'f':
        this->filename_ = get_opt.opt_arg ();
        break;
      case 'l':
        this->length_ = ACE_OS::atoi (get_opt.opt_arg ());
        break;
      case 'o':
        this->offset_ = ACE_OS::atoi (get_opt.opt_arg ());
        break;
      default: // DEBUG模式下失配时，需要输出LOG。
        ACE_DEBUG ((LM_DEBUG, "%s -h hostname -f filename -[r/w] [-p port] [-l length] [-o offset] [-d]\n", argv[0]));
        ACE_OS::exit (1);
      }
  if (this->hostname_ == 0 || this->filename_ == 0) // hostname和filename不可不输入
    {
      ACE_DEBUG ((LM_DEBUG,
                  "%s -h hostname -f filename -[r/w] [-p port] [-l length] [-o offset] [-d]\n",
                  argv[0]));

      ACE_OS::exit (1);
    }

}

Options::Options () // 构造器给结构体赋初值。
  : hostname_ (0),
    port_ (ACE_DEFAULT_HTTP_SERVER_PORT),
    filename_ (0),
    length_ (0),
    offset_ (0),
    operation_ ('r'),
    debug_ (0)
{
}