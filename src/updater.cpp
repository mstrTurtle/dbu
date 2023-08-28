#include "updater.h"

#include "ace/Init_ACE.h"
#include "ace/Log_Msg.h"
#include "downloader.h"
#include "ftp_operation.h"
#include "installer.h"
#include "option.h"
#include "sniffer.h"
#include "sniffer_errors.h"

int Updater::run()
{
    int err;

    // 初始化ACE
    ACE::init();

    // ftp控制连接的sock工厂
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    Sniff_Hint hint = convert_option_to_sniff_hint(option);

    std::cout << "Option parse succeed. Listed below:\n" << (option);

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    // run sniffer，查找最新版本的product

    Sniffer sniffer(addr, sock, hint);

    string path;
    if ((err = sniffer.run(path))) {
        std::cout << "Sniffer encounter error, error code:" << err
                  << std::endl;
    }

    std::cout << "Sniffer done, got path: " << path << std::endl;

    // run downloader, 多线程下载
    Downloader downloader(
            path, OPTION->threads_, "download.rpm", sock, sock_creator);

    if (downloader.run()) {
        return 1;
    }

    // run installer，安装
    Installer installer;
    if (installer.run()) {
        std::cout << "Install error.\n";
        ACE_DEBUG((LM_ERROR, "%p\n", "updater.run() error"));
        return 1;
    }
    // 关闭ACE
    ACE::fini();

    return 0;
}
