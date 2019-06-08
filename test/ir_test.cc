#include <gtest/gtest.h>

#include "visitor/ir/context.h"
#include "visitor/ir/type_checker.h"

using namespace juicyc::llvm_ir;
using namespace llvm;

TEST(IRTypeCheckerTest, StringRep) {
  Context context;
  TypeChecker checker(&context);
  InternalTypePtr type = checker.FromString("int");
  EXPECT_TRUE(type->llvm != nullptr);
  InternalTypePtr p = type->pointer();
  EXPECT_TRUE(p->llvm != nullptr);
  EXPECT_EQ(p->id, InternalTypeID::kPointer);
  EXPECT_EQ(p->ToString(), "IntegerTy*");
  InternalTypePtr arr = type->array(10);
  EXPECT_TRUE(arr->llvm != nullptr);
  EXPECT_EQ(arr->id, InternalTypeID::kArray);
  EXPECT_EQ(arr->ToString(), "IntegerTy[10]");
  InternalTypePtr arr_to_arr = arr->array(100);
  EXPECT_TRUE(arr_to_arr->llvm != nullptr);
  EXPECT_EQ(arr_to_arr->id, InternalTypeID::kArray);
  EXPECT_EQ(arr_to_arr->ToString(), "IntegerTy[10][100]");
}