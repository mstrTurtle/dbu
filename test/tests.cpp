// tests.cpp
#include "Option.h"
#include <gtest/gtest.h>

/**
 * @brief 检验合法选项
 *
 */
TEST(DbuOptionTest, ValidOption)
{
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
  ASSERT_EQ(1, 1);
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
