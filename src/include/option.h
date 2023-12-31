/**
 * @file option.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief 解析命令行选项的类
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include "ace/ARGV.h"
#include <string>

/**
 * @class Option
 * @brief Option类用于解析命令行参数并存储选项值
 */
class Option
{
public:
    /**
     * @brief 获取Option类的唯一实例
     * @return Option类的唯一实例指针
     */
    static Option* instance();

    /**
     * @brief 解析命令行参数并设置选项值
     * @param argc 命令行参数数量
     * @param argv 命令行参数数组
     */
    void parse_args(int argc, ACE_TCHAR* argv[]);

    std::string branch_;
    std::string sub_branch_;
    std::string build_;
    std::string arch_;
    std::string product_;
    int threads_;
    bool debug_;

    friend std::ostream& operator<<(std::ostream& os, const Option& option);

protected:
    /**
     * @brief Option类的构造函数
     */
    Option();

    /**
     * @brief Option类的唯一实例指针
     */
    static Option* instance_;
};

#define OPTION (Option::instance())
