#include "sniffer.h"
#include <gtest/gtest.h>

using std::string;

/**
 * @brief 检验正确的路径
 *
 */
TEST(DbuSnifferTest, ValidPath)
{
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    Sniff_Hint hint{
            .branch = "hotfix",
            .subbranch = "56006-hana",
            .option = "debug",
            .arch = "x86_64",
            .product = "gaussdb"};

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    // run sniffer，查找最新版本的product

    Sniffer sniffer(addr, sock, hint);

    string path;
    int ret = sniffer.run(path);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(
            path,
            "/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/8.0.35618/"
            "dbackup3-agent-gaussdb-8.0.35618-1.6586071.dbg.x86_64.rpm");
}

/**
 * @brief 检验错误的路径
 *
 */
TEST(DbuSnifferTest, InvalidPath)
{
    ACE_INET_Addr addr(21, "ftp.scutech.com");
    Sock_Creator sock_creator =
            make_logined_sock_creator(addr, "scutech", "dingjia");

    Sniff_Hint hint{
            .branch = "hotfix",
            .subbranch = "56006-hana",
            .option = "release",
            .arch = "x86_64",
            .product = "gaussdb"};

    // 连接到端点
    SOCK sock;
    sock_creator(sock);

    // run sniffer，查找最新版本的product

    Sniffer sniffer(addr, sock, hint);

    string path;
    int ret = sniffer.run(path);

    ASSERT_NE(ret, 0);
}


int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
