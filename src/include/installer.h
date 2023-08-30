/**
 * @file updater.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief Updater程序入口
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <string>

class Installer
{
    std::string path;

public:
    [[nodiscard]] int run();
};
