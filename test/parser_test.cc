#include <gtest/gtest.h>

#include "mock_env.h"
#include "juicyc/system.h"
#include "juicyc/compiler.h"

using namespace juicyc;

class ParserTest : public testing::Test {
 public:
  ParserTest() { Init(); }
  ~ParserTest() {}
  void Init() {
    options_.isys = &isys_;
    options_.osys = &osys_;
  }
  test::MockInputSystem isys_;
  test::MockOutputSystem osys_;
  CompilerOptions options_;
};

TEST_F(ParserTest, VariableDecl) {
  isys_.set_file("main.c", "int a = 1*9-8;");
  options_.files.push_back("main.c");
  options_.dump_output = "main.json";
  auto compiler = Compiler::NewCompiler(options_);
  EXPECT_TRUE(compiler->Parse().ok());
  std::cout << osys_.get_file("main.json");
}

TEST_F(ParserTest, FunctionDecl) {

}