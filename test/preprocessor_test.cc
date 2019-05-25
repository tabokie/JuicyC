#include <gtest/gtest.h>

#include "juicyc/preprocessor.h"
#include "mock_env.h"

using namespace juicyc;

TEST(PreprocessorTest, ReadLine) {
  test::MockEnv env;
  auto sys = reinterpret_cast<test::MockInputSystem*>(env.input_system());
  std::string content = "first line\nsecond line\n";
  sys->set_file("main.cc", content);
  auto p = Preprocessor::NewPreprocessor(CompilerOption(), &env);
  p->push(std::string("main.cc"));
  EXPECT_TRUE(p->seek());
  char buf[50];
  int cur = 0;
  while (true) {
    buf[cur] = p->get();
    ASSERT_TRUE(p->ok());
    if (!p->eof()) cur ++;
    else break;  // cur is '\0'
  }
  EXPECT_EQ(content, std::string(buf, cur));
}

TEST(PreprocessorTest, ConditionalMacro) {
  test::MockEnv env;
  auto sys = reinterpret_cast<test::MockInputSystem*>(env.input_system());
  std::string content = "#define A\n#ifdef A\nI should exist.\n#define B\n#endif\n#ifndef B\nI should not exist.\n#endif\n";
  sys->set_file("main.cc", content);
  auto p = Preprocessor::NewPreprocessor(CompilerOption(), &env);
  p->push(std::string("main.cc"));
  EXPECT_TRUE(p->seek());
  char buf[50];
  int cur = 0;
  while (true) {
    buf[cur] = p->get();
    ASSERT_TRUE(p->ok());
    if (!p->eof()) cur ++;
    else break;  // cur is '\0'
  }
  EXPECT_EQ("I should exist.\n", std::string(buf, cur));
}

TEST(PreprocessorTest, IncludeHeader) {
  test::MockEnv env;
  auto sys = reinterpret_cast<test::MockInputSystem*>(env.input_system());
  std::string content = "#include \"header.h\"\nperiod.\n";
  sys->set_file("main.cc", content);
  content = "I have a ";
  sys->set_file("header.h", content);
  auto p = Preprocessor::NewPreprocessor(CompilerOption(), &env);
  p->push(std::string("main.cc"));
  EXPECT_TRUE(p->seek());
  char buf[50];
  int cur = 0;
  while (true) {
    buf[cur] = p->get();
    ASSERT_TRUE(p->ok());
    if (!p->eof()) cur ++;
    else break;  // cur is '\0'
  }
  // notice that pp will automatically append '\n'
  EXPECT_EQ("I have a \nperiod.\n", std::string(buf, cur));
}

TEST(PreprocessorTest, ContextDescriptor) {
  test::MockEnv env;
  auto sys = reinterpret_cast<test::MockInputSystem*>(env.input_system());
  std::string content = "#include \"header.h\"\n$\n";
  sys->set_file("main.cc", content);
  content = "-#\n---*\n";
  sys->set_file("header.h", content);
  auto p = Preprocessor::NewPreprocessor(CompilerOption(), &env);
  p->push(std::string("main.cc"));
  EXPECT_TRUE(p->seek());
  while (!p->eof()) {
    ASSERT_TRUE(p->ok());
    if (p->get() == '$') {
      EXPECT_EQ(p->file_name(), "main.cc");
      EXPECT_EQ(p->line_no(), 2);
      EXPECT_EQ(p->col_no(), 1);
    } else if (p->get() == '#') {
      EXPECT_EQ(p->file_name(), "header.h");
      EXPECT_EQ(p->line_no(), 1);
      EXPECT_EQ(p->col_no(), 2);
    } else if (p->get() == '*') {
      EXPECT_EQ(p->file_name(), "header.h");
      EXPECT_EQ(p->line_no(), 2);
      EXPECT_EQ(p->col_no(), 4);
    }
  }
}