#include "type_checker.h"

namespace juicyc {
namespace syntax {

std::unordered_map<std::string, std::function<llvm::Type*(llvm::LLVMContext&)>>
    TypeChecker::llvm_type_string_map_ = {
          {"int", llvm::Type::getInt32Ty},
          {"float", llvm::Type::getFloatTy},
          {"char", llvm::Type::getInt8Ty},
          {"double", llvm::Type::getDoubleTy},
          {"void", llvm::Type::getVoidTy},
          {"bool", llvm::Type::getInt1Ty},
          {"int8_t", llvm::Type::getInt8Ty},
          {"int16_t", llvm::Type::getInt16Ty},
          {"int32_t", llvm::Type::getInt32Ty},
          {"int64_t", llvm::Type::getInt64Ty}};

}  // namespace syntax
}  // namespace juicyc