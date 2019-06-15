#include <gtest/gtest.h>

#include "mock_env.h"
#include "juicyc/system.h"
#include "juicyc/compiler.h"

using namespace juicyc;

bool emit_result = true;

class ParserTest : public testing::Test {
 public:
  ParserTest() { Init(); }
  ~ParserTest() {}
  void Init() {
    options_.isys = &isys_;
    options_.osys = &osys_;
  }
  void Test(std::string content,
            bool enable_analyse = true,
            bool enable_compile = true) {
    isys_.set_file("main.c", content);
    options_.files.push_back("main.c");
    options_.json_output = "main.json";
    if (enable_analyse) {
      options_.ir_output = "main.llvm";
    }
    if (enable_compile) {
      options_.obj_output = "tmp.obj";  // just to test compilation      
    }
    auto compiler = Compiler::NewCompiler(options_);
    EXPECT_TRUE(compiler->Run().ok());
    if (emit_result) {
      // std::cout << "=== JSON ===" << std::endl;
      // std::cout << osys_.get_file("main.json") << std::endl;
      if (enable_analyse) {
        std::cout << "=== LLVM ===" << std::endl;
        std::cout << osys_.get_file("main.llvm") << std::endl;  
      }  
    }
  }
  void DarkTest(std::string content, std::string ir) {
    return;
    isys_.set_file("main.c", content);
    options_.files.push_back("main.c");
    options_.json_output = "main.json";
    auto compiler = Compiler::NewCompiler(options_);
    EXPECT_TRUE(compiler->Run().ok());
    if (emit_result) {
      std::cout << "=== JSON ===" << std::endl;
      std::cout << osys_.get_file("main.json") << std::endl;
      std::cout << "=== LLVM ===" << std::endl;
      std::cout << ir << std::endl;  
    }
  }

  test::MockInputSystem isys_;
  test::MockOutputSystem osys_;
  CompilerOptions options_;
};

TEST_F(ParserTest, VariableDecl) {
  Test("int a = 1*9-8;");
}

TEST_F(ParserTest, FunctionDecl) {
  Test("void func() {} int main(int a) { return 1;}");
}

TEST_F(ParserTest, VariableAssignment) {
  Test("int a = 1+1; int main(void) {a += a; return 1;}");
}

TEST_F(ParserTest, ExternPrintf) {
  // Test("extern int printf(const char*, ...);");
  Test("extern int printf(const char*, ...); int main(void) { printf(\"hello world\\n\"); return 0;}");
}

TEST_F(ParserTest, FunctionCall) {
  Test("int func(void) { return 0; } int main(void) { int a = func(); return 0; }");
}

TEST_F(ParserTest, OverloadedFunctionCall) {
  Test(std::string("int func(void) { return 0; }") +
      "int func(int a) { return 1; }" +
      "int func(int a, float b) { return 2; }" +
      "int main(void) { int a = func(); int b = func(1); int c = func(1,1); return 0;}");
}

TEST_F(ParserTest, IfElseClause) {
  Test("int main(void) { int a = 1; if (a == 0) { a=2; } else { a=3; } return a;}");
}

TEST_F(ParserTest, DISABLED_Sample1) {
  Test("int i;\n\
int go(int a) {\n\
  if (a == 1) {\n\
    return 1;\n\
  }\n\
  if (a == 2) {\n\
    return 1;\n\
  }\n\
  return go(a-1) + go(a-2);\n\
}\n\
int main(void) {\n\
  i = func(10);\n\
  return 0;\n\
}");
}

TEST_F(ParserTest, Sample2) {
  Test("extern int printf(const char*, int);\
int f;\n\
int k;\n\
int go(int b, int a) {\n\
  int ret;\n\
  int fk;\n\
  float t;\n\
  if (a > 0) {\n\
    ret = a * go(b, a-1);\n\
  } else {\n\
    ret = 1;\n\
  }\n\
  b += ret;\n\
  k += ret;\n\
  return ret;\n\
}\n\
int main(void) {\n\
  k = 0;\n\
  f = go(k, 5);\n\
  printf(\"%d\\n\", f);\n\
  printf(\"%d\\n\", k);\n\
  return 0;\n\
}");
}

TEST_F(ParserTest, Sample3) {
  Test("extern int printf(const char*, int);\
int ans;\
int gcd(int a, int b) {\
  if (b == 0) {\
    return a;\
  } else {\
    return gcd(b, a % b);\
  }\
}\
int main(void) {\
  ans = gcd(9, 36) * gcd(3, 6);\
  printf(\"%d\", ans);\
  return 0;\
}");
}
