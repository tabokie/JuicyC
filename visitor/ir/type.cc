#include "type.h"

namespace juicyc {
namespace llvm_ir {

std::map<llvm::Type::TypeID, std::string>
    InternalType::llvm_type_to_string_ = {
          {llvm::Type::VoidTyID, "VoidTy"},
          {llvm::Type::HalfTyID, "HalfTy"},
          {llvm::Type::FloatTyID, "FloatTy"},
          {llvm::Type::DoubleTyID, "DoubleTy"},
          {llvm::Type::IntegerTyID, "IntegerTy"},
          {llvm::Type::FunctionTyID, "FunctionTy"},
          {llvm::Type::StructTyID, "StructTy"},
          {llvm::Type::ArrayTyID, "ArrayTy"},
          {llvm::Type::PointerTyID, "PointerTy"},
          {llvm::Type::VectorTyID, "VectorTy"}};

InternalTypePtr InternalType::pointer() {
  return std::make_shared<Pointer>(shared_from_this());
}

InternalTypePtr InternalType::array(uint32_t count) {
  return std::make_shared<Array>(shared_from_this(), count);
}

}  // namespace llvm_ir
}  // namespace juicyc