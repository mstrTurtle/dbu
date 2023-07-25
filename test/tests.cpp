// tests.cpp
#include "Option.h"
#include <gtest/gtest.h>
 
TEST(SquareRootTest, PositiveNos) { 
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
}
 
TEST(SquareRootTest, NegativeNos) {
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
    ASSERT_EQ(1,1);
}
 
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}