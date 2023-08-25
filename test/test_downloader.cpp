/**
 * @file downloader_test.cpp
 * @brief Downloader 类的单元测试文件
 */

#include "downloader.h"
#include <gtest/gtest.h>

/**
 * @brief 测试单线程下载
 *
 * 测试使用单线程下载文件是否成功。
 */
TEST(DbuInstallerTest, SingleThread)
{
    // ftp控制连接的sock工厂
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    Downloader downloader(
            "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/8.0.35618/"
            "dbackup3-agent-gaussdb-8.0.35618-1.6586071.dbg.x86_64.rpm",
            1, "download.rpm", sock, sock_creator);
    int ret = downloader.run();
    ASSERT_EQ(ret, 0);
}

/**
 * @brief 测试多线程下载
 *
 * 测试使用多线程下载文件是否成功。
 */
TEST(DbuInstallerTest, MultiThread)
{
    // ftp控制连接的sock工厂
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    Downloader downloader(
            "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/8.0.35618/"
            "dbackup3-agent-gaussdb-8.0.35618-1.6586071.dbg.x86_64.rpm",
            4, "download.rpm", sock, sock_creator);
    int ret = downloader.run();
    ASSERT_EQ(ret, 0);
}

/**
 * @brief 测试错误的路径
 *
 * 测试使用错误的文件路径下载文件是否成功。
 */
TEST(DbuInstallerTest, BadPath)
{
    // ftp控制连接的sock工厂
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    // 构造错误的路径，'hotfix/1000'是不存在的。
    Downloader downloader(
            "/ftp_product_installer/dbackup3/rpm/hotfix/1000/8.0.35618/"
            "dbackup3-agent-gaussdb-8.0.35618-1.6586071.dbg.x86_64.rpm",
            1, "download.rpm", sock, sock_creator);
    int ret = downloader.run();
    ASSERT_EQ(ret, 0);
}

int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
