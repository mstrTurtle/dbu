// tests.cpp
#include "Option.h"
#include <gtest/gtest.h>

/**
 * @brief 检验合法选项
 *
 */
TEST(DbuOptionTest, ValidOption)
{
  const char* argv[] = {
    "dbu",   "-d", "-b",     "hotfix", "-s",       "56006-hana", "-u",
    "debug", "-a", "x86_64", "-p",     "informix", "-t",         "4",
  };

  auto option = Option::instance();
  option->parse_args(sizeof(argv) / sizeof(const char*), (char**)argv);
  auto path = option->get_actual_path();
  ASSERT_EQ("/ftp_product_installer/dbackup3/rpm/hotfix/56006-hana/debug/"
            "x86_64/informix",
            path);
}

/**
 * @brief 检验非法选项
 *
 */
TEST(DbuOptionTest, InvalidOption)
{
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
}

/**
 * @brief 初始化并且RUN_ALL_TESTS
 *
 * @param argc
 * @param argv
 * @return int
 */
int
main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

// int main(){
//     const char* argv[] = {
//     "dbu",   "-d", "-b",     "hotfix", "-s",       "56006-hana", "-u",
//     "debug", "-a", "x86_64", "-p",     "informix", "-t",         "4",
//   };

//   auto option = Option::instance();
//   option->parse_args(sizeof(argv) / sizeof(const char*), (char**)argv);
//   auto path = option->get_actual_path();
//   // ASSERT_EQ("", path);
// }
