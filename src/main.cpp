#include "Downloader.h"
#include "Option.h"
#include "Sniffer.h"
#include "Installer.h"
#include "ace/Log_Msg.h"

int
ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
  OPTION->parse_args(argc, argv);
  SniffHint hint = convertOptionToSniffHint(*OPTION);

  ACE_INET_Addr addr(21, "ftp.vim.org");

  SOCK sock;

  ACE_SOCK_Connector connector;
  if (connector.connect(sock, addr) == -1) {
    ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
    return 1;
  }

  Sniffer sniffer(addr,sock,hint);

  string path;
  sniffer.run(path);

  Downloader downloader("/pub/robots.txt",OPTION->threads_, "robots.txt");

  downloader.run();

  // Updater updater;
  // if (updater.run(argc, argv) == -1)
  //   ACE_ERROR_RETURN((LM_ERROR, "%p\n", "updater.run()"), 1);

  return 0;
}
