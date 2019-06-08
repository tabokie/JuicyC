#include "type_checker.h"

namespace juicyc {
namespace llvm_ir {

std::unordered_map<std::string, std::function<Type*(LLVMContext&)>>
    TypeChecker::llvm_type_string_map_ = {
          {"int", Type::getInt32Ty},
          {"float", Type::getFloatTy},
          {"char", Type::getInt8Ty},
          {"double", Type::getDoubleTy},
          {"void", Type::getVoidTy},
          {"bool", Type::getInt1Ty},
          {"int8_t", Type::getInt8Ty},
          {"int16_t", Type::getInt16Ty},
          {"int32_t", Type::getInt32Ty},
          {"int64_t", Type::getInt64Ty}};

}  // namespace llvm_ir
}  // namespace juicyc