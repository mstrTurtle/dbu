#include "Installer.h"
#include "Downloader.h"
#include "Option.h"
#include "ace/Log_Msg.h"

#include "ace/Process.h"
#include <iostream>

/**
 * @brief 执行安装脚本
 *
 * @return int
 */
int Installer::run()
{
    // 构造命令
    ACE_Process_Options options;
    options.command_line(
            ACE_TEXT("%") ACE_TEXT_PRIs ACE_TEXT(" -i") ACE_TEXT("%")
                    ACE_TEXT_PRIs,
            "rpm", "download.rpm");
    // spawn新进程
    ACE_Process new_process;
    if (new_process.spawn(options) == -1) {
        int error_number = ACE_OS::last_error();
        ACE_ERROR(
                (LM_ERROR, ACE_TEXT("%p errno = %d.\n"), ACE_TEXT("install"),
                 error_number));
    }

    // 检查是否正确执行
    ACE_exitcode status;
    new_process.wait(&status);
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Process exit with status %d\n"), status));
    return 0;
}
