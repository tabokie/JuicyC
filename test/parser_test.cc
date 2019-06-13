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
      std::cout << "=== JSON ===" << std::endl;
      std::cout << osys_.get_file("main.json") << std::endl;
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
/*
  DarkTest("int a = 1*9-8;", "; ModuleID = \'tmp.c\'\n\
source_filename = \"tmp.c\"\n\
target datalayout = \"e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32\"\n\
target triple = \"i686-pc-windows-msvc19.0.24210\"\n\
\n\
@a = dso_local global i32 1, align 4");
*/
}

TEST_F(ParserTest, FunctionDecl) {
  Test("void func() {} int main(int a) { return 1;}", true, false);
}

TEST_F(ParserTest, VariableAssignment) {
  Test("int a = 1+1; int main(void) {a += a; return 1;}", true, false);
}

TEST_F(ParserTest, DISABLED_FunctionVariable) {
  Test("void main(int a) { int b = 0; { int a = 0; } }");
}

TEST_F(ParserTest, DarkTestSample) {
  DarkTest("int i;\n\
int go(int a) {\n\
  if (a == 1) {\n\
    return 1;\n\
  } else {\n\
    if (a == 2) {\n\
      return 1;\n\
    } else {\n\
      return go(a-1) + go(a-2);\n\
    }\n\
  }\n\
}\n\
int main(void) {\n\
  i = go(10);\n\
  return 0;\n\
}", "; ModuleID = \'main.c\'\n\
source_filename = \"main.c\"\n\
target datalayout = \"e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32\"\n\
target triple = \"i686-pc-windows-msvc19.0.24210\"\n\
\n\
@i = common dso_local global i32 0, align 4\n\
\n\
; Function Attrs: noinline nounwind optnone\n\
define dso_local i32 @go(i32) #0 {\n\
  %2 = alloca i32, align 4\n\
  %3 = alloca i32, align 4\n\
  store i32 %0, i32* %3, align 4\n\
  %4 = load i32, i32* %3, align 4\n\
  %5 = icmp eq i32 %4, 1\n\
  br i1 %5, label %6, label %7\n\
\n\
; <label>:6:                                      ; preds = %1\n\
  store i32 1, i32* %2, align 4\n\
  br label %19\n\
\n\
; <label>:7:                                      ; preds = %1\n\
  %8 = load i32, i32* %3, align 4\n\
  %9 = icmp eq i32 %8, 2\n\
  br i1 %9, label %10, label %11\n\
\n\
; <label>:10:                                     ; preds = %7\n\
  store i32 1, i32* %2, align 4\n\
  br label %19\n\
\n\
; <label>:11:                                     ; preds = %7\n\
  %12 = load i32, i32* %3, align 4\n\
  %13 = sub nsw i32 %12, 1\n\
  %14 = call i32 @go(i32 %13)\n\
  %15 = load i32, i32* %3, align 4\n\
  %16 = sub nsw i32 %15, 2\n\
  %17 = call i32 @go(i32 %16)\n\
  %18 = add nsw i32 %14, %17\n\
  store i32 %18, i32* %2, align 4\n\
  br label %19\n\
\n\
; <label>:19:                                     ; preds = %11, %10, %6\n\
  %20 = load i32, i32* %2, align 4\n\
  ret i32 %20\n\
}\n\
\n\
; Function Attrs: noinline nounwind optnone\n\
define dso_local i32 @main() #0 {\n\
  %1 = alloca i32, align 4\n\
  store i32 0, i32* %1, align 4\n\
  %2 = call i32 @go(i32 10)\n\
  store i32 %2, i32* @i, align 4\n\
  ret i32 0\n\
}");
}

TEST_F(ParserTest, DISABLED_Printf) {
  Test("extern printf(const char*); void main() { printf(\"hello world.\\n\"); }");
}

TEST_F(ParserTest, DISABLED_Case2) {
  Test("extern printf(const char*);\
int i;\
int go(int a) {\
  if (a == 1) {\
    return 1;\
  } else {\
    if (a == 2) {\
      return 1;\
    } else {\
      return go(a-1) + go(a-2);\
    }\
  }\
}\
void main(void) {\
  i = go(10);\
  printf(\"%d\", i);\
  return;\
}");
}

TEST_F(ParserTest, DISABLED_Case4) {
  Test("extern printf(const char*);\
int f;\
int k;\
int go(int b, int a) {\
  int ret;\
  int fk;\
  float t;\
  if (a > 0) {\
    ret = a * go(b, a-1);\
  } else {\
    ret = 1;\
  }\
  b += ret;\
  k += ret;\
}\
void main(void) {\
  k = 0;\
  f = go(k, 5);\
  printf(\"%d\n\", f);\
  printf(\"%d\n\", k);\
  return;\
}");
}

TEST_F(ParserTest, DISABLED_Case6) {
  Test("extern printf(const char*);\
int ans;\
int gcd(int a, int b) {\
  if (b == 0) {\
    return a;\
  } else {\
    return gcd(b, a % b);\
  }\
}\
void main(void) {\
  ans = gcd(9, 36) * gcd(3, 6);\
  printf(\"%d\", ans);\
  return;\
}");
}