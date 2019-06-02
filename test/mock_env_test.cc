#include <gtest/gtest.h>

#include "mock_env.h"
#include "juicyc/system.h"

using namespace juicyc;

TEST(MockSystemTest, BasicRead) {
  test::MockInputSystem sys;
  sys.set_file("test_file.a", "hello world");
  IStreamPtr s (sys.fopen("test_file.a"));
  char buf[20];
  memset(buf, 0, sizeof(char) * 20);
  s->getline(buf, 20);
  EXPECT_EQ(std::string(buf), std::string("hello world"));
}

TEST(MockSystemTest, BasicWrite) {
  test::MockOutputSystem sys;
  OStreamPtr s(sys.fopen("test_file.b"));
  (*s) << "hello world";
  sys.fclose(s);
  EXPECT_EQ(sys.get_file("test_file.b"), "hello world");
}