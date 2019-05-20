#include <gtest/gtest.h>

#include "mock_system.h"
#include "juicyc/system.h"

#include <istream>

using juicyc::test::MockInputSystem;
using juicyc::test::MockOutputSystem;

TEST(MockSystemTest, BasicRead) {
	MockInputSystem sys;
	sys.set_file("test_file.a", "hello world");
	juicyc::IStreamPtr s (sys.fopen("test_file.a"));
	char buf[20];
	memset(buf, 0, sizeof(char) * 20);
	s->getline(buf, 20);
	EXPECT_EQ(std::string(buf), std::string("hello world"));
}


TEST(MockSystemTest, BasicWrite) {
	MockOutputSystem sys;
	juicyc::OStreamPtr s(sys.fopen("test_file.b"));
	(*s) << "hello world";
	sys.fclose(s);
	EXPECT_EQ(sys.get_file("test_file.b"), "hello world");
}