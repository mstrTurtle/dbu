#include "Downloader.h"
#include "Option.h"
#include "Sniffer.h"
#include "Updater.h"
#include "ace/Log_Msg.h"

int
ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{

  // Sniffer sniffer;
  // sniffer.run();

  Downloader downloader("/pub/robots.txt",1, "robots.txt");

  downloader.run();

  // Updater updater;
  // if (updater.run(argc, argv) == -1)
  //   ACE_ERROR_RETURN((LM_ERROR, "%p\n", "updater.run()"), 1);

  return 0;
}
