// #include "installer.h"
#include <gtest/gtest.h>


/**
 * @brief 平凡测试
 *
 */
TEST(InstallerTest, Ordinary)
{
    // ASSERT_EQ(0,install());
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
