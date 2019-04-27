#include <gtest/gtest.h>

#include "util/file.h"
#include "util.h"

using namespace juicyc;
using test::rnd;

TEST(FileTest, Basic) {
  constexpr size_t file_size = 1024;
  SequentialFile file("_filetest_0");
  ASSERT_TRUE(file.Open().ok());
  ASSERT_TRUE(file.Close().ok());
  ASSERT_TRUE(file.Delete().ok()); // only delete if exists
  ASSERT_TRUE(file.Open().ok());
  char data[file_size];
  char fill = rnd.Int(128);
  for(int i = 0; i < file_size; i++) {
    data[i] = fill;
  }
  ASSERT_TRUE(file.SetEnd(file_size).ok());
  ASSERT_TRUE(file.Write(0, file_size, data).ok());
  ASSERT_TRUE(file.Close().ok());
  ASSERT_TRUE(file.Open().ok());
  ASSERT_TRUE(file.size() == file_size);
  ASSERT_TRUE(file.Read(0, file_size, data).ok());
  for(int i = 0; i < file_size; i++) {
    EXPECT_EQ(data[i], fill);
  }
  ASSERT_TRUE(file.Close().ok());
  ASSERT_TRUE(file.Delete().ok());
}