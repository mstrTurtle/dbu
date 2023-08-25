#include "sniffer.h"
#include "ftp_operation.h"
#include "ftp_util.h"
#include "option.h"
#include "sniffer_errors.h"
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

using namespace Sniffer_Errors;

int Sniffer::process_branch()
{
    std::cout << "In " << __func__ << "\n";
    if (hint.branch == "develop" || hint.branch == "master") {
        join_path(cwd, hint.branch);
        join_path(cwd, hint.subbranch);
        return OK;
    } else if (
            hint.branch == "feature" || hint.branch == "hotfix" ||
            hint.branch == "support") {
        join_path(cwd, hint.branch);
        bool found;
        if (fetch_find(conn.sock, cwd, hint.subbranch, found)) {
            return BAD_FETCH;
        }
        if (!found) {
            return BAD_LOOKUP;
        }
        join_path(cwd, hint.subbranch);
        return OK;
    }
    return OK;
}

int Sniffer::process_option()
{
    std::cout << "In " << __func__ << "\n";
    bool found;
    if (fetch_find(conn.sock, cwd, hint.option, found)) {
        return BAD_FETCH;
    }
    if (!found) {
        return BAD_LOOKUP;
    }
    join_path(cwd, hint.option);
    return OK;
}

int Sniffer::process_target()
{
    std::cout << "In " << __func__ << "\n";
    bool found;
    if (fetch_find(conn.sock, cwd, hint.arch, found)) {
        ACE_DEBUG((LM_ERROR, "网络有问题.\n"));
        return BAD_FETCH;
    }
    if (!found) {
        ACE_DEBUG((LM_ERROR, "你提供的arch信息是错的.\n"));
    }
    join_path(cwd, hint.arch);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return OK;
}

int Sniffer::process_version()
{
    std::cout << "In " << __func__ << "\n";
    string result;
    fetch_find_max(conn.sock, cwd, result);
    join_path(cwd, result);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return OK;
}

int Sniffer::process_functionality()
{
    std::cout << "In " << __func__ << "\n";
    VS v;
    if (fetch_fzf(conn.sock, cwd, hint.product, v)) {
        return BAD_FETCH;
    }
    if (v.size() < 1) {
        return BAD_FETCH;
    }
    string target;
    get_regular_name(v[0], target);
    join_path(cwd, target);
    std::cout << __func__ << " completed, cwd is" << cwd << std::endl;
    return OK;
}

int Sniffer::run(string& result)
{
    cwd = "/ftp_product_installer/dbackup3/rpm";

    int err = 0;

    if ((err = process_branch()) != 0) {
        std::cout << "process_branch 函数出错，状态码：" << err << std::endl;
    } else if ((err = process_option()) != 0) {
        std::cout << "process_option 函数出错，状态码：" << err << std::endl;
    } else if ((err = process_target()) != 0) {
        std::cout << "process_target 函数出错，状态码：" << err << std::endl;
    } else if ((err = process_version()) != 0) {
        std::cout << "process_version 函数出错，状态码：" << err << std::endl;
    } else if ((err = process_functionality()) != 0) {
        std::cout << "process_functionality 函数出错，状态码：" << err
                  << std::endl;
    }

    if (err) {
        std::cout << "探测失败\n";
        return BAD_LOOKUP;
    }

    std::cout << "探测成功，目标是：" << cwd << std::endl;

    result = cwd;
    return OK;
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
