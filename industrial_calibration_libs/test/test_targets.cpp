#include <industrial_calibration_libs/industrial_calibration_libs.h>
#include <gtest/gtest.h>
#include "test_utils.h"

TEST(Targets, temp_test)
{
  CONSOLE_OUTPUT("ABCDEFG");
  EXPECT_TRUE(industrial_calibration_libs::number == 5);
  CONSOLE_OUTPUT("Hello World!");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}