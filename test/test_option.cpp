/**
 * @file option_test.cpp
 * @brief Option 类的单元测试文件
 */

#include "option.h"
#include <gtest/gtest.h>

/**
 * @brief 检验合法选项
 *
 * 测试合法选项是否被正确解析和设置。
 */
TEST(DbuOptionTest, ValidOption)
{
  const char* argv[] = {
    "dbu",   "-d", "-b",     "hotfix", "-s",       "56006-hana", "-u",
    "debug", "-a", "x86_64", "-p",     "informix", "-t",         "4",
  };

  auto option = Option::instance();
  option->parse_args(sizeof(argv) / sizeof(const char*), (char**)argv);

  ASSERT_EQ(option->branch_, "hotfix");
  ASSERT_EQ(option->sub_branch_, "x86_64");
  ASSERT_EQ(option->build_, "debug");
  ASSERT_EQ(option->arch_, "x86_64");
  ASSERT_EQ(option->product_, "informix");
  ASSERT_EQ(option->threads_, 4);
}

int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
