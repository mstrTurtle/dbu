#include "Option.h"
#include "Updater.h"
#include "ace/Log_Msg.h"
#include "Sniffer.h"
#include "Downloader.h"

int
ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{

  // Sniffer sniffer;
  // sniffer.run();

  Downloader downloader;
  downloader.run("");

  Updater updater;
  if (updater.run(argc, argv) == -1)
    ACE_ERROR_RETURN((LM_ERROR, "%p\n", "updater.run()"), 1);

  return 0;
}
