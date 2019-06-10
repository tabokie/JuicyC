#ifndef JUICYC_VISITOR_SYNTAX_STACKABLE_CONTEXT_H_
#define JUICYC_VISITOR_SYNTAX_STACKABLE_CONTEXT_H_

#include "identifier.h"

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <string>
#include <unordered_map>

namespace juicyc {
namespace syntax {

class IdentifierTable {
 public:
	bool Insert(const Identifier& identifier) {
		if (table_.find(identifier.name) != table_.end()) {
			return false;
		} else {
			table_[identifier.name] = identifier;
			return true;
		}
	}
	bool Get(std::string name, Identifier& ret) const {
		auto& p = table_.find(name);
		if (p == table_.end()) {
			return false;
		} else {
			ret = p->second;
			return true;
		}
	}
	Identifier& operator[](std::string name) {
		auto& p = table_.find(name);
		return p->second;
	}
	const Identifier& operator[](std::string name) const {
		auto& p = table_.find(name);
		return p->second;
	}
 private:
 	std::unordered_map<std::string, Identifier> table_;;
};

struct StackableContext {
	llvm::BasicBlock* llvm;
	llvm::Value* value;

	IdentifierTable identifier;

	StackableContext() = default;
	~StackableContext() {}
};

struct GlobalContext {
	IdentifierTable identifier;

	GlobalContext() = default;
	~GlobalContext() {}
};

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_STACKABLE_CONTEXT_H_