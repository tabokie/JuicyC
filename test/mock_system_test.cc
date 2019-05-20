#include <gtest/gtest.h>

#include "mock_system.h"
#include "juicyc/system.h"

#include <istream>

using juicyc::test::MockInputSystem;

TEST(MockSystemTest, BasicRead) {
	MockInputSystem sys;
	sys.set_file("test_file.a", "hello world");
	juicyc::IStreamPtr s (sys.fopen("test_file.a"));
	char buf[20];
	memset(buf, 0, sizeof(char) * 20);
	s->getline(buf, 20);
	EXPECT_EQ(std::string(buf), std::string("hello world"));
}