#ifndef JUICYC_VISITOR_SYNTAX_TYPE_H_
#define JUICYC_VISITOR_SYNTAX_TYPE_H_

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/Casting.h>

#include <string>
#include <cstdint>
#include <memory>
#include <map>
#include <iostream>

namespace juicyc {
namespace syntax {

enum InternalTypeID {
  kPrimitive = 1,
  kPointer = 2,
  kArray = 3,
  kStruct = 4
};

// Bind with identifier, immutable.
struct InternalType : std::enable_shared_from_this<InternalType> {
  llvm::Type* llvm = nullptr;
  InternalTypeID id = kPrimitive;

  InternalType() = default;
  InternalType(llvm::Type* t) : llvm(t) {}
  InternalType(const InternalType& rhs)
      : std::enable_shared_from_this<InternalType>(rhs), 
        llvm(rhs.llvm), id(rhs.id) {}
  InternalType(InternalType&& rhs)
      : std::enable_shared_from_this<InternalType>(std::move(rhs)),
        llvm(rhs.llvm), id(rhs.id) {}
  virtual std::string ToString() {
    if (!llvm) return "NilTy";
    auto p = llvm_type_to_string_.find(llvm->getTypeID());
    if (p != llvm_type_to_string_.end()) return p->second;
    return "UnknownTy";
  }
  virtual std::shared_ptr<InternalType> pointer();
  virtual std::shared_ptr<InternalType> array(uint32_t count);
  virtual llvm::Value* default_value() {
    auto _id = llvm->getTypeID();
    if (_id == llvm::Type::IntegerTyID) {
      return llvm::ConstantInt::get(llvm, 0, true);
    } else if(_id == llvm::Type::FloatTyID ||
              _id == llvm::Type::DoubleTyID) {
      return llvm::ConstantFP::get(llvm, 0);
    } else {
      return nullptr;
    }
  }
  static std::map<llvm::Type::TypeID, std::string>
      llvm_type_to_string_;
};

using InternalTypePtr = std::shared_ptr<InternalType>;

struct Pointer : public InternalType {
  InternalTypePtr pointee = nullptr;

  Pointer(InternalTypePtr p) : pointee(p) {MakeType();}
  Pointer(const Pointer& rhs) : InternalType(rhs), pointee(rhs.pointee) {}
  Pointer(Pointer&& rhs) : InternalType(rhs), pointee(rhs.pointee) {}
  std::string ToString() override {
    return pointee->ToString() + "*";
  }
  llvm::Value* default_value() override {
    return llvm::ConstantPointerNull::get(
        llvm::dyn_cast<llvm::PointerType>(llvm));
  }
 private:
  void MakeType() {
    id = kPointer;
    if (pointee) {
      llvm = llvm::PointerType::get(pointee->llvm, 0);
    }
  }
};

struct Array : public InternalType {
  uint32_t count;
  InternalTypePtr pointee = nullptr;

  Array(InternalTypePtr p, uint32_t n) : pointee(p), count(n) {MakeType();}
  Array(const Array& rhs) : InternalType(rhs), count(rhs.count), pointee(rhs.pointee) {}
  Array(Array&& rhs) : InternalType(rhs), count(rhs.count), pointee(rhs.pointee) {}
  std::string ToString() override {
    return pointee->ToString() + "[" + std::to_string(count) + "]";
  }
  llvm::Value* default_value() override {
    return nullptr;
  }
 private:
  void MakeType() {
    id = kArray;
    if (pointee) {
      llvm = llvm::ArrayType::get(pointee->llvm, count);
    }
  }
};

// struct Struct : public InternalType {

// };

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_TYPE_H_