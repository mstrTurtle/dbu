#include "downloader.h"
#include "ftp_operation.h"
#include "installer.h"
#include "option.h"
#include "sniffer.h"
#include "updater.h"
#include "ace/Log_Msg.h"

int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
    OPTION->parse_args(argc, argv);
    Updater updater(*OPTION);
    if(int ret = updater.run()){
        std::cout << "Error occured in updater. error code: " << ret;
    }
    return 0;
}
