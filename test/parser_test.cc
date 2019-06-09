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
    options_.json_output = "main.json";
    options_.ir_output = "main.llvm";
    auto compiler = Compiler::NewCompiler(options_);
    EXPECT_TRUE(compiler->Run().ok());
    std::cout << "=== JSON ===" << std::endl;
    std::cout << osys_.get_file("main.json") << std::endl;
    std::cout << "=== LLVM ===" << std::endl;
    std::cout << osys_.get_file("main.llvm") << std::endl;
  }
  test::MockInputSystem isys_;
  test::MockOutputSystem osys_;
  CompilerOptions options_;
};

TEST_F(ParserTest, VariableDecl) {
  Test("int a = 1*9-8;");
}

TEST_F(ParserTest, DISABLED_FunctionDecl) {
  Test("void main() {}");
}