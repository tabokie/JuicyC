#ifndef JUICYC_VISITOR_SYNTAX_IDENTIFIER_H_
#define JUICYC_VISITOR_SYNTAX_IDENTIFIER_H_

#include "context.h"
#include "type.h"

#include <iostream>
#include <memory>

namespace juicyc {
namespace syntax {

struct Identifier : std::enable_shared_from_this<Identifier> {
  std::string name;
  InternalTypePtr type = nullptr;
  llvm::Value* value = nullptr;

  Identifier() = default;
  Identifier(std::string _name, InternalTypePtr _type, llvm::Value* _value)
      : name(_name),
        type(_type),
        value(_value) {}
  Identifier(const Identifier& rhs)
      : name(rhs.name),
        type(rhs.type),
        value(rhs.value) {}
  // Indexers returns value of ptr
  // index by data member
  llvm::Value* index(Context* context, std::string field) {
    if (type->id == kStruct) {
      // auto p = reinterpret_cast<Struct*>(type.get());
      // uint32_t idx = p->index(field);
      uint32_t idx = 0;
      std::vector<llvm::Value*> v {
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm), 0, false),
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm), idx, false)};
      llvm::ArrayRef<llvm::Value*> indices(v);
      return context->builder.CreateInBoundsGEP(value, indices);
    }
    return nullptr;
  }
  // index by fixed indice
  llvm::Value* index(Context* context, uint32_t value) {
    return index(context,llvm::ConstantInt::get(
               llvm::Type::getInt64Ty(context->llvm),
               value));
  }
  // index by variable indice
  llvm::Value* index(Context* context, llvm::Value* value) {
    llvm::ArrayRef<llvm::Value*> indices;
    if (type->id == kPointer) {
      std::vector<llvm::Value*> v {
          llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm),0),
          value};
      indices = v;
      return context->builder.CreateInBoundsGEP(value, indices);
    } else if (type->id == kArray) {
      std::vector<llvm::Value*> v { value };
      indices = v;
      return context->builder.CreateInBoundsGEP(
          context->builder.CreateLoad(value), indices);
    }
    return nullptr;
  }

  std::shared_ptr<Identifier> shared() { return shared_from_this(); }
};

using IdentifierPtr = std::shared_ptr<Identifier>;

struct FunctionIdentifier {
  std::string name;
  std::string tag;
  InternalTypePtr ret = nullptr;
  std::vector<InternalTypePtr> args;
  llvm::Value* value;
  bool closed = false;

  FunctionIdentifier(std::string _name,
                     InternalTypePtr _ret,
                     std::vector<InternalTypePtr> _args,
                     llvm::Value* _value)
      : name(_name),
        ret(_ret),
        args(_args),
        value(_value) {}
  FunctionIdentifier(const FunctionIdentifier& rhs)
      : name(rhs.name),
        tag(rhs.tag),
        ret(rhs.ret),
        args(rhs.args),
        value(rhs.value),
        closed(rhs.closed) {}
  ~FunctionIdentifier() {}
};

using FunctionIdentifierPtr = std::shared_ptr<FunctionIdentifier>;

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_IDENTIFIER_H_