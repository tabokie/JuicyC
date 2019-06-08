#ifndef JUICYC_VISITOR_IR_CONTEXT_H_
#define JUICYC_VISITOR_IR_CONTEXT_H_

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace juicyc {
namespace llvm_ir {

struct Context {
	llvm::LLVMContext llvm;
	llvm::IRBuilder<> builder;
	Context() : builder(llvm) {}
};

}  // namespace llvm_ir
}  // namespace juicyc

#endif  // JUICYC_VISITOR_IR_CONTEXT_H_