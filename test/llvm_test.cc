#include <gtest/gtest.h>

#include "llvm/ADT/APFloat.h"

using namespace llvm;

// dummy test to test llvm environment
TEST(LLVMTest, PrimitiveType) {
  auto a = APFloat(2.0);
  auto b = APFloat(1.0);
  a = a + b;
  EXPECT_EQ(a.convertToDouble(), 3.0);  // caution: not to float
}