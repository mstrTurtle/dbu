#include "sniffer.h"
#include "ftp_operation.h"
#include "ftp_util.h"
#include "option.h"
#include "cstring"
#include <ace/INET_Addr.h>
#include <ace/Init_ACE.h>
#include <ace/Log_Msg.h>
#include <ace/Log_Priority.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

using std::string;
using std::vector;

using VS = vector<string>;

// 函数用于判断主分支并处理分支
int Sniffer::process_branch()
{
    std::cout << "In " << __func__ << "\n";
    if (hint.branch == "develop" || hint.branch == "master") {
        join_path(cwd, hint.branch);
        join_path(cwd, hint.subbranch);
        return 0;
    } else if (
            hint.branch == "feature" || hint.branch == "hotfix" ||
            hint.branch == "support") {

        join_path(cwd, hint.branch);
        bool b = fetch_find(conn.sock, cwd, hint.subbranch);
        if (!b) {
            return 2;
        }
        join_path(cwd, hint.subbranch);
        return 0;
    }
    return 0;
}

/**
 * @brief 处理选项
 *
 * @return int
 */
int Sniffer::process_option()
{
    std::cout << "In " << __func__ << "\n";
    if(!fetch_find(conn.sock, cwd, hint.option)){
        return 1;
    }
    join_path(cwd, hint.option);
    return 0;
}

/**
 * @brief 处理目标
 *
 * @return int
 */
int Sniffer::process_target()
{
    std::cout << "In " << __func__ << "\n";
    if (!fetch_find(conn.sock, cwd, hint.arch)) {
        ACE_DEBUG((LM_ERROR, "你提供的arch信息是错的.\n"));
        return 1;
    }
    join_path(cwd, hint.arch);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return 0;
}

/**
 * @brief 处理版本号
 *
 * @return int
 */
int Sniffer::process_version()
{
    std::cout << "In " << __func__ << "\n";
    Str result;
    fetch_find_max(conn.sock, cwd, result);
    join_path(cwd, result);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return 0;
}

/**
 * @brief 处理功能
 *
 * @return int
 */
int Sniffer::process_functionality()
{
    std::cout << "In " << __func__ << "\n";
    VS v;
    if (fetch_fzf(conn.sock, cwd, hint.product, v)) {
        return 1;
    }
    if (v.size() < 1) {
        return 2;
    }
    Str target;
    get_regular_name(v[0], target);
    join_path(cwd, target);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return 0;
}

/**
 * 嗅探的入口。传入Option，逐步嗅探。返回错误码。
 * TODO: 我觉得得定义一下错误码。
 */
int Sniffer::run(Str& result)
{
    cwd = "/ftp_product_installer/dbackup3/rpm";

    int err = 0;

    if ((err = process_branch()) != 0) {
        std::cout << "process_branch 函数出错，状态码：" << err << std::endl;
    } else if ((err = process_option()) != 0) {
        std::cout << "process_option 函数出错，状态码：" << err << std::endl;
        exit(1);
    } else if ((err = process_target()) != 0) {
        std::cout << "process_target 函数出错，状态码：" << err << std::endl;
        exit(1);
    } else if ((err = process_version()) != 0) {
        std::cout << "process_version 函数出错，状态码：" << err << std::endl;
        exit(1);
    } else if ((err = process_functionality()) != 0) {
        std::cout << "process_functionality 函数出错，状态码：" << err << std::endl;
        exit(1);
    }

    if(err){
        std::cout << "退出程序\n";
        exit(1);
    }

    std::cout << "探测成功，目标是：" << cwd << std::endl;


    result = cwd;
    return 0;
}

int test_main()
{
    // 初始化ACE
    ACE::init();

    // 建立控制连接
    ACE_SOCK_Stream control_socket;
    ACE_INET_Addr control_addr("ftp.vim.org", "21");
    ACE_SOCK_Connector connector;
    if (connector.connect(control_socket, control_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Error connecting to control socket.\n"));
        return 1;
    }

    // 登录
    setup_control(control_socket);

    Sniffer sniffer(control_addr, control_socket, {});
    Str result;
    sniffer.run(result);

    // 关闭ACE
    ACE::fini();

    return 0;
}

Sniff_Hint convert_option_to_sniff_hint(const Option& option)
{
    Sniff_Hint sniffHint = {
            .branch = option.branch_,
            .subbranch = option.sub_branch_,
            .option = option.build_,
            .arch = option.arch_,
            .product = option.product_};
    return sniffHint;
}
