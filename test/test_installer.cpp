/**
 * @file installer_test.cpp
 * @brief Installer 类的单元测试文件
 */

#include "installer.h"
#include <gtest/gtest.h>

/**
 * @brief 检验合法安装包
 *
 * 测试安装包是否正确安装并返回 0。
 */
TEST(DbuInstallerTest, ValidPackage)
{
  Installer installer;
  int ret = installer.run();
  ASSERT_EQ(ret, 0);
}

int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
