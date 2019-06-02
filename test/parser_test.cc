#include <gtest/gtest.h>

#include "mock_env.h"
#include "juicyc/system.h"
#include "juicyc/compiler.h"

using namespace juicyc;

TEST(ParserTest, MockSystem) {
  test::MockInputSystem isys;
  test::MockOutputSystem osys;
  isys.set_file("main.c", "int a = 0;");

	CompilerOptions opts;
	opts.isys = &isys;
	opts.osys = &osys;
	opts.files.push_back("main.c");
	opts.dump_output = "main.json";

	auto compiler = Compiler::NewCompiler(opts);
	EXPECT_TRUE(compiler->Parse().ok());
	std::cout << osys.get_file("main.json");
}