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
  Status Insert(const IdentifierPtr& identifier) {
    if (table_.find(identifier->name) != table_.end()) {
      return Status::Corruption("duplicate");
    } else {
      table_[identifier->name] = identifier;
      return Status::OK();
    }
  }
  Status Get(const std::string name, IdentifierPtr& ret) const {
    auto& p = table_.find(name);
    if (p == table_.end()) {
      return Status::NotFound("identifier");
    } else {
      ret = p->second;
      return Status::OK();
    }
  }
  IdentifierPtr& operator[](std::string name) {
    auto& p = table_.find(name);
    return p->second;
  }
  const IdentifierPtr& operator[](std::string name) const {
    auto& p = table_.find(name);
    return p->second;
  }
 private:
  std::unordered_map<std::string, IdentifierPtr> table_;
};

class FunctionTable {
 public:
  FunctionTable(TypeChecker* checker) : checker_(checker) {}
  Status Insert(FunctionIdentifierPtr& identifier) {
    if (identifier->tag.size() == 0) {
      identifier->tag = checker_->Tag(identifier->args);
    }
    auto& v = table_[identifier->name];
    for (auto& f : v) {
      if (f->tag == identifier->tag) {
        return Status::Corruption("duplicate");
      }
    }
    v.push_back(identifier);
    return Status::OK();
  }

  Status Get(const std::string name,
             const std::vector<InternalTypePtr>& args,
             FunctionIdentifierPtr& ret) {
    std::string tag = checker_->Tag(args);
    auto& v = table_[name];
    for (auto& f : v) {
      if (f->tag == tag) {
        ret = f;
        return Status::OK();
      }
    }
    int args_count = args.size();
    int find_castable = -1;
    for (int n = 0; n < v.size(); n++) {
      auto& f = v[n];
      if (f->args.size() == args_count) {
        int i;
        for (i = 0; i < args_count; i++) {
          if (!checker_->Castable(args[i]->llvm, f->args[i]->llvm)) {
            break;
          }
        }
        if (i == args_count) {
          if (find_castable >= 0) {
            return Status::Corruption("ambiguous match");
          } else {
            find_castable = n;
          }
        }
      }
    }
    if (find_castable < 0) {
      return Status::NotFound("function");
    } else {
      ret = v[find_castable];
      return Status::OK();
    }
  }

 private:
  std::unordered_map<std::string, std::vector<FunctionIdentifierPtr>> table_;
  TypeChecker* checker_;
};

struct StackableContext {
  llvm::BasicBlock* llvm;
  FunctionIdentifierPtr hook;

  IdentifierTable identifier;
  FunctionTable function;

  StackableContext(TypeChecker* checker) : function(checker) {}
  ~StackableContext() {}
};

struct GlobalContext {
  IdentifierTable identifier;
  FunctionTable function;

  GlobalContext(TypeChecker* checker) : function(checker) {}
  ~GlobalContext() {}
};

}  // namespace syntax
}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_STACKABLE_CONTEXT_H_