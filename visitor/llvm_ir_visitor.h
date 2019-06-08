#ifndef JUICYC_VISITOR_LLVM_IR_VISITOR_H_
#define JUICYC_VISITOR_LLVM_IR_VISITOR_H_

#include "juicyc/status.h"
#include "ir/type_checker.h"
#include "ir/context.h"
#include "util/util.h"
#include "frontend/front_env.h"

#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>

#include <iostream>

namespace juicyc {

class LLVMIRVisitor : public SymbolVisitor {
 public:
  LLVMIRVisitor() {};
  ~LLVMIRVisitor() {}
  void VisitTerminal(Terminal* n) {
    FunctionGuard g(std::cout, std::string("VisitTerminal:") + n->value);
    switch (n->type) {
      case CONSTANT:
      if (n->value.find('.') != std::string::npos) {
        double v = atof(n->value.c_str());
        value_stack_.push_back(ConstantFP::get(Type::getDoubleTy(context_.llvm), v));
      } else {
        std::cout << __LINE__ << std::endl;
        int v = atoi(n->value.c_str());
        std::cout << __LINE__ << std::endl;
        value_stack_.push_back(ConstantInt::get(Type::getInt32Ty(context_.llvm), v, true));
        std::cout << __LINE__ << std::endl;
      }
      break;
      case STRING_LITERAL:
      value_stack_.push_back(context_.builder.CreateGlobalStringPtr(n->value, "const char*"));
      break;
      default:
      break;
    }
  }
  void ExitTerminal(Terminal* n) {}
  void VisitNonTerminal(NonTerminal* n) {
    FunctionGuard g(std::cout, std::string("VisitNonTerminal:") + FrontEnv::Untag(n->type));
  }
  void ExitNonTerminal(NonTerminal* n) {}
  bool VisitUnaryExpression(UnaryExpression* expr) {
    return true;
  }
  void ExitUnaryExpression(UnaryExpression* expr) {
    // case INC_OP:
    // case DEC_OP:
    // case SIZEOF:
    // case SIZEOF:
    // // unary_operator:
    // case '&':
    // case '*':
    // case '+':
    // case '-':
    // case '~':
    // case '!':
  }
  bool VisitBinaryExpression(BinaryExpression* expr) {
    FunctionGuard g(std::cout, "VisitBinaryExpression");
    return true;
  }
  void ExitBinaryExpression(BinaryExpression* expr) {
    FunctionGuard g(std::cout, "ExitBinaryExpression");
    if (expr && expr->childs->right && value_stack_.size() >= 2) {  // exit two operands
      FunctionGuard l(std::cout, "Calc");
      Symbol* op = expr->childs->right;
      Value* b = value_stack_.back();
      value_stack_.erase(value_stack_.end()-1);
      Value* a = value_stack_.back();
      value_stack_.erase(value_stack_.end()-1);
      bool fp = (a->getType()->getTypeID() == llvm::Type::DoubleTyID ||
                 a->getType()->getTypeID() == llvm::Type::FloatTyID ||
                 a->getType()->getTypeID() == llvm::Type::DoubleTyID ||
                 a->getType()->getTypeID() == llvm::Type::FloatTyID);
      Value* ret = nullptr;
      switch (op->type) {
        case '*': ret = fp ? context_.builder.CreateFMul(a, b)
                           : context_.builder.CreateMul(a, b);
        break;
        case '/': ret = fp ? context_.builder.CreateFDiv(a, b)
                           : context_.builder.CreateSDiv(a, b);
        break;
        case '%': ret = fp ? nullptr
                           : context_.builder.CreateSRem(a, b);
        break;
        case '+': ret = fp ? context_.builder.CreateFAdd(a, b)
                           : context_.builder.CreateAdd(a, b);
        break;
        case '-': ret = fp ? context_.builder.CreateFSub(a, b)
                           : context_.builder.CreateSub(a, b);
        break;
        case LEFT_OP: ret = fp ? nullptr
                               : context_.builder.CreateShl(a, b);
        break;
        case RIGHT_OP: ret = fp ? nullptr
                                : context_.builder.CreateAShr(a, b);
        break;
        case '<': fp ? context_.builder.CreateFCmpULT(a, b)
                     : context_.builder.CreateICmpULT(a, b);
        break;
        case '>': fp ? context_.builder.CreateFCmpOGT(a, b)
                     : context_.builder.CreateICmpSGT(a, b);
        break;
        case LE_OP: fp ? context_.builder.CreateFCmpOLE(a, b)
                       : context_.builder.CreateICmpSLE(a, b);
        break;
        case GE_OP: fp ? context_.builder.CreateFCmpOGE(a, b)
                       : context_.builder.CreateICmpSGE(a, b);
        break;
        case EQ_OP: fp ? context_.builder.CreateFCmpOEQ(a, b)
                       : context_.builder.CreateICmpEQ(a, b);
        break;
        case NE_OP: fp ? context_.builder.CreateFCmpONE(a, b)
                       : context_.builder.CreateICmpNE(a, b);
        break;
        case '&': ret = fp ? nullptr
                           : context_.builder.CreateAnd(a, b);
        break;
        case '^': ret = fp ? nullptr
                           : context_.builder.CreateXor(a, b);
        break;
        case '|': ret = fp ? nullptr
                           : context_.builder.CreateOr(a, b);
        break;
        // case AND_OP:
        // case OR_OP:
        default:
        break;
      }
      if (ret) {
        value_stack_.push_back(ret);
      }
    }
  }
  Status status() const override { return status_; }
 protected:
  Status status_;
  llvm_ir::Context context_;
  llvm_ir::TypeChecker type_checker_;
  std::vector<Value*> value_stack_;
  // FunctionTable functions_;
  // IdentifierTable identifiers_;
};

}  // namespace juicyc

#endif  // JUICYC_VISITOR_LLVM_IR_VISITOR_H_