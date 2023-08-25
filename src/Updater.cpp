#include "Updater.h"
#include "Sniffer.h"
#include "downloader.h"
#include "ftp_operation.h"
#include "installer.h"
#include "Option.h"
#include "Sniffer.h"
#include "ace/Log_Msg.h"

int Updater::run()
{
    // ftp控制连接的sock工厂
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    SockCreator sockCreator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    SniffHint hint = convertOptionToSniffHint(option);

    std::cout << (option);

    // 连接到端点
    SOCK sock;
    sockCreator(sock);

    // run sniffer，查找最新版本的product

    Sniffer sniffer(addr, sock, hint);

    string path;
    sniffer.run(path);

    std::cout << "Sniffer done, got path: " << path << std::endl;
    exit(0);

    // run downloader, 多线程下载
    Downloader downloader(
            "/pub/robots.txt", OPTION->threads_, "download.rpm", sock,
            sockCreator);

    downloader.run();

    // run installer，安装
    Installer installer;
    if (installer.run() == -1)
        ACE_ERROR_RETURN((LM_ERROR, "%p\n", "updater.run() error"), 1);

    return 0;
}
