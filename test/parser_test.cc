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
  void Test(std::string content) {
    isys_.set_file("main.c", content);
    options_.files.push_back("main.c");
    options_.dump_output = "main.json";
    auto compiler = Compiler::NewCompiler(options_);
    EXPECT_TRUE(compiler->Parse().ok());
    std::cout << osys_.get_file("main.json");
  }
  test::MockInputSystem isys_;
  test::MockOutputSystem osys_;
  CompilerOptions options_;
};

TEST_F(ParserTest, VariableDecl) {
  Test("int a = 1*9-8;");
}

TEST_F(ParserTest, FunctionDecl) {
  Test("void main() {}");
}