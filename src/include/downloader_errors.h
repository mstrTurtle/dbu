/**
 * @file downloader_errors.h
 * @author JiayuXiao (xiaojiayu@scutech.com)
 * @brief Downloader模块的错误代码枚举类
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
namespace Downloader_Errors
{
    enum Downloader_Errors
    {
        OK = 0,
        BAD_PATH = 1,
        BAD_SOCKET = 2,
        BAD_FILE_HANDLE = 3,
    };
}
