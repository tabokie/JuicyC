#ifndef JUICYC_VISITOR_IR_TYPE_CHECKER_H_
#define JUICYC_VISITOR_IR_TYPE_CHECKER_H_

#include "context.h"
#include "type.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <unordered_map>
#include <map>
#include <functional>
#include <string>

using namespace llvm;

namespace juicyc {
namespace llvm_ir {

class TypeChecker {
 public:
  TypeChecker() = default;
  TypeChecker(Context* context) : context_(context) {}
  ~TypeChecker() {}
  void set_context(Context* context) { context_ = context; }
  InternalTypePtr FromString(std::string str) {
    auto p = llvm_type_string_map_.find(str);
    if (p == llvm_type_string_map_.end()) return nullptr;
    return std::make_shared<InternalType>(p->second(context_->llvm));
  }
  // type cast is only applied on raw value and type
  Value* CastTo(Value* value, Type* to, BasicBlock* block, bool _explicit = true) {
    Type* from = value->getType();
    if (from == to) return value;
    if (_explicit) {
      // cross cast
      if (from->isPointerTy() && to->isPointerTy()) {
        return context_->builder.CreateBitCast(value, to);
      }
      // pointer integer cast
      if(from->isPointerTy() && to->isIntegerTy()) {
        return context_->builder.CreatePtrToInt(value, to);
      } else if(from->isIntegerTy() && to->isPointerTy()) {
        return context_->builder.CreateIntToPtr(value, to);
      }
      // array pointer cast
      if (from->isPointerTy() && to->isArrayTy() ||
          from->isArrayTy() && to->isPointerTy()) {
        return context_->builder.CreateBitCast(value, to);
      }
      // no struct cast allowed
      if (from->isStructTy() || to->isStructTy()) {
        return nullptr;
      }
    }
    llvm::Type::TypeID from_id = from->getTypeID();
    llvm::Type::TypeID to_id = to->getTypeID();
    if (from_id == llvm::Type::IntegerTyID) {
      return context_->builder.CreateSExtOrTrunc(value, to);
    } else if(from_id == llvm::Type::FloatTyID) {
      if (to_id == llvm::Type::DoubleTyID)
        return context_->builder.CreateFPExt(value, to);
      else
        return context_->builder.CreateFPTrunc(value, to);
    }
  }
  Value* CastToBoolean(Value* value) {
    Type* type = value->getType();
    llvm::Type::TypeID id = type->getTypeID();
    if (id == llvm::Type::IntegerTyID) {
      Value* condValue = context_->builder.CreateIntCast(value, Type::getInt1Ty(context_->llvm), true);
      return context_->builder.CreateICmpNE(condValue, ConstantInt::get(Type::getInt1Ty(context_->llvm), 0, true));
    } else if (id == llvm::Type::FloatTyID || id == llvm::Type::DoubleTyID) {
      return context_->builder.CreateFCmpONE(value, ConstantFP::get(context_->llvm, APFloat(0.0)));
    }
    return nullptr;
  }
 private:
  Context* context_ = nullptr;
  static std::unordered_map<std::string, std::function<Type*(LLVMContext&)>>
      llvm_type_string_map_;
};

}  // namespace llvm_ir
}  // namespace juicyc

#endif  // JUICYC_VISITOR_IR_TYPE_CHECKER_H_