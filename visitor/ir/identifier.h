#ifndef JUICYC_VISITOR_IR_IDENTIFIER_H_
#define JUICYC_VISITOR_IR_IDENTIFIER_H_

#include "context.h"
#include "type.h"

namespace juicyc {
namespace llvm_ir {

struct Identifier {
	uint32_t scope;
	std::string name;
	Value* value;
	InternalTypePtr type;
	// Indexers returns value of ptr
	// index by data member
	Value* index(Context* context, std::string field) {
		if (type.id == kStruct) {
			auto& p = reinterpret_cast<std::shared_ptr<Struct>>(type);
			uint32_t idx = type->index(field);
			llvm::ArrayRef<Value*> indices = {
				  llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm), 0, false),
				  llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm), idx, false)};
		  return context->builder.CreateInBoundsGEP(value, indices);
		}
		return nullptr;
	}
	// index by fixed indice
	Value* index(Context* context, uint32_t value) {
		return index(context,llvm::ConstantInt::get(
			         llvm::Type::getInt64Ty(context->llvm),
			         value));
	}
	// index by variable indice
	Value* index(Context* context, Value* value) {
		llvm::ArrayRef<Value*> indices;
		if (type.id == kPointer) {
			indices = {
				  llvm::ConstantInt::get(llvm::Type::getInt64Ty(context->llvm),0),
			    value};
			return context->builder.CreateInBoundsGEP(value, indices);
		} else if (type.id == kArray) {
			indices = {value};
			return context->builder.CreateInBoundsGEP(
				  context->builder.CreateLoad(value), indices);
		}
		return nullptr;
	}
};

}  // namespace llvm_ir
}  // namespace juicyc

#endif  // JUICYC_VISITOR_IR_IDENTIFIER_H_