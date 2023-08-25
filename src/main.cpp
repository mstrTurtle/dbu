#include "downloader.h"
#include "ftp_operation.h"
#include "installer.h"
#include "Option.h"
#include "Sniffer.h"
#include "Updater.h"
#include "ace/Log_Msg.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    OPTION->parse_args(argc, argv);
    Updater updater(*OPTION);
    updater.run();
    return 0;
}
