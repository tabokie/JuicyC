#ifndef JUICYC_VISITOR_SYNTAX_TYPE_CHECKER_H_
#define JUICYC_VISITOR_SYNTAX_TYPE_CHECKER_H_

#include "context.h"
#include "type.h"
#include "util/util.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <unordered_map>
#include <map>
#include <functional>
#include <string>
#include <iostream>

namespace juicyc {
namespace syntax {

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
  llvm::Value* CastTo(llvm::Value* value, llvm::Type* to, bool _explicit = true) {
    llvm::Type* from = value->getType();
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
  llvm::Value* CastToBoolean(llvm::Value* value) {
    llvm::Type* type = value->getType();
    llvm::Type::TypeID id = type->getTypeID();
    if (id == llvm::Type::IntegerTyID) {
      llvm::Value* condValue = context_->builder.CreateIntCast(value, llvm::Type::getInt1Ty(context_->llvm), true);
      return context_->builder.CreateICmpNE(condValue,
          llvm::ConstantInt::get(llvm::Type::getInt1Ty(context_->llvm), 0, true));
    } else if (id == llvm::Type::FloatTyID || id == llvm::Type::DoubleTyID) {
      return context_->builder.CreateFCmpONE(value,
          llvm::ConstantFP::get(context_->llvm, llvm::APFloat(0.0)));
    }
    return nullptr;
  }
  bool Castable(const llvm::Type* from, const llvm::Type* to) {
    if (from == to) return true;
    llvm::Type::TypeID from_id = from->getTypeID();
    llvm::Type::TypeID to_id = to->getTypeID();
    if (from_id == llvm::Type::IntegerTyID ||
        from_id == llvm::Type::FloatTyID && to_id != llvm::Type::IntegerTyID) {
      return true;
    }
    return false;
  }
  std::string Tag(const std::vector<InternalTypePtr>& args) {
    return "";
  }
 private:
  Context* context_ = nullptr;
  static std::unordered_map<std::string, std::function<llvm::Type*(llvm::LLVMContext&)>>
      llvm_type_string_map_;
};

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_TYPE_CHECKER_H_