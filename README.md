# dbackup-updater

迪备客户端更新命令行程序

使用Adaptive Communication Environment编写

## 详情

使用 ACE 实现迪备客户端更新命令行工具（HF 项目）开发环境

- OS：CentOS 7
- 编辑器：VIM
- 编译器：GCC (Linux)
- 构建：CMake
- VCS：Git
- 开发语言： C++ 、CMake 构建、Git 版本管理、 gtest 单元测试、Doxygen 对代码注释

## 规格

1. 使用 C++ 实现一个控制台执行程序
2. 基于开源网络库 ACE(ADAPTIVE Communication Environment)搭建程序框架
3. 根据输入参数，比如代码分支名称，资源类型（比如指定 DB2 和 Informix），到
ftp://scutech@ftp.scutech.com/ftp_product_installer/dbackup3/rpm 下载最新版本的客户端程序。
注意下级目录的命名方式：分支类型、分支名称、debug 或 release、CPU 架构、当前版本号，比如hotfix/bug-60697/debug/x86_64/8.0.35276
4. 要求可以指定并发下载的线程数，多线程下载
5. 每个功能模块需要有 gtest 单元测试、Doxygen 对代码注释
6. 执行新安装或升级操作
7. 给出完善的报错信息

参考 GB8567-2006 撰写开发文档，包括：

- 概要设计
- 详细设计
