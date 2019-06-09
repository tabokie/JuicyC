#ifndef JUICYC_VISITOR_SYNTAX_VISITOR_H_
#define JUICYC_VISITOR_SYNTAX_VISITOR_H_

#include "juicyc/status.h"
#include "juicyc/env.h"
#include "syntax/type_checker.h"
#include "syntax/context.h"
#include "syntax/identifier.h"
#include "syntax/stackable_context.h"
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

class SyntaxVisitor : public SymbolVisitor {
 public:
  SyntaxVisitor(Env* env) {
    context_.env = env;
    type_checker_.set_context(&context_);
  };
  ~SyntaxVisitor() {}
  void VisitTerminal(Terminal* n) {
    FunctionGuard g(std::cout, std::string("VisitTerminal:") + n->value);
    context_.logger.trace(std::string("VisitTerminal:") + n->value);
    // context_.logger.set_cursor(FrontEnv::Untag(n->file) + ":" +
                               // std::to_string(n->line) + ":" +
                               // std::to_string(n->col));
    switch (n->type) {
      case CONSTANT:
      if (n->value.find('.') != std::string::npos) {
        double v = atof(n->value.c_str());
        value_stack_.push_back(llvm::ConstantFP::get(
            llvm::Type::getDoubleTy(context_.llvm), v));
      } else {
        int v = atoi(n->value.c_str());
        value_stack_.push_back(llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context_.llvm), v, true));
      }
      break;
      case STRING_LITERAL:
      value_stack_.push_back(context_.builder.CreateGlobalStringPtr(
          n->value, "const char*"));
      break;
      default:
      break;
    }
  }
  void ExitTerminal(Terminal* n) {}
  void VisitNonTerminal(NonTerminal* n) {
    context_.logger.trace(std::string("VisitNonTerminal:") +
        FrontEnv::Untag(n->type));
  }
  void ExitNonTerminal(NonTerminal* n) {}
  bool VisitRoot(Root* root) {
    context_.logger.trace(std::string("VisitRoot"));
    context_stack_.push_back(syntax::StackableContext());
    std::vector<llvm::Type*> args;
    llvm::FunctionType* type = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context_.llvm),
        llvm::ArrayRef<llvm::Type*>(args),
        false);
    llvm::Function* function = llvm::Function::Create(
        type,
        llvm::GlobalValue::ExternalLinkage,
        "main",
        context_.module.get());
    llvm::BasicBlock* block = llvm::BasicBlock::Create(
        context_.llvm, "entry", function);
    context_.builder.SetInsertPoint(block);
    context_stack_.back().llvm = block;
    return true;
  }

  /* Expression */

  bool VisitAssignmentExpression(AssignmentExpression* expr) {
    context_.logger.trace(std::string("VisitAssignmentExpression"));
    // = conditional_expression
    // = unary_expression assignment_operator assignment_expression
    return true;
  }

  bool VisitTernaryExpression(TernaryExpression* expr) {
    context_.logger.trace(std::string("VisitTernaryExpression"));
    // = binary_expression
    // = binary ? expression : ternary_expression
    return true;
  }

  bool VisitBinaryExpression(BinaryExpression* expr) {
    context_.logger.trace(std::string("VisitBinaryExpression"));
    // = binary_expression OP cast_expression
    return true;
  }
  void ExitBinaryExpression(BinaryExpression* expr) {
    FunctionGuard g(std::cout, "ExitBinaryExpression");
    if (expr && expr->childs->right && value_stack_.size() >= 2) {
      FunctionGuard l(std::cout, "Calc");
      Symbol* op = expr->childs->right;
      llvm::Value* b = value_stack_.back();
      value_stack_.pop_back();
      llvm::Value* a = value_stack_.back();
      value_stack_.pop_back();
      bool fp = 
          (a->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           a->getType()->getTypeID() == llvm::Type::FloatTyID ||
           a->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           a->getType()->getTypeID() == llvm::Type::FloatTyID);
      llvm::Value* ret = a;  // wrong but usable
      switch (op->type) {
        case '*': ret = fp ? context_.builder.CreateFMul(a, b)
                           : context_.builder.CreateMul(a, b);
        break;
        case '/': ret = fp ? context_.builder.CreateFDiv(a, b)
                           : context_.builder.CreateSDiv(a, b);
        break;
        case '%': if (fp) context_.logger.error(
                              "Mod operation on floating point");
                  else ret = context_.builder.CreateSRem(a, b);
        break;
        case '+': ret = fp ? context_.builder.CreateFAdd(a, b)
                           : context_.builder.CreateAdd(a, b);
        break;
        case '-': ret = fp ? context_.builder.CreateFSub(a, b)
                           : context_.builder.CreateSub(a, b);
        break;
        case LEFT_OP: if (fp) context_.logger.error(
                                  "Left shift operation on floating point");
                      else ret = context_.builder.CreateShl(a, b);
        break;
        case RIGHT_OP: if (fp) context_.logger.error(
                                   "Right shift operation on floating point");
                       else ret = context_.builder.CreateAShr(a, b);
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
        case '&': if (fp) context_.logger.error(
                              "And operation on floating point");
                  else ret = context_.builder.CreateAnd(a, b);
        break;
        case '^': if (fp) context_.logger.error(
                              "Xor operation on floating point");
                  else ret = context_.builder.CreateXor(a, b);
        break;
        case '|': if (fp) context_.logger.error(
                              "Or operation on floating point");
                  else ret = context_.builder.CreateOr(a, b);
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

  bool VisitUnaryExpression(UnaryExpression* expr) {
    context_.logger.trace(std::string("VisitUnaryExpression"));
    // = postfix_expression
    // = OP unary_expression/cast_expression
    // = SIZEOF ( TYPE )
    return true;
  }
  void ExitUnaryExpression(UnaryExpression* expr) {
    /*
    case INC_OP:
    case DEC_OP:
    case SIZEOF:
    case SIZEOF:
    // -> unary_operator
    case '&':
    case '*':
    case '+':
    case '-':
    case '~':
    case '!':
    */
  }

  /* Declaration */

  bool VisitDeclaration(Declaration* decl) {
    FunctionGuard g(std::cout, "VisitDeclaration");
    // = declaration_spec ;
    // = declaration_spec init_declarator ;
    if (!decl->childs->right->is_terminal) {
      // declaration with init_declarator
    } else {
      context_.logger.error(
          "Unimplemented: declaration={declaration_specifiers}");
    }
    return true;
  }
  void ExitDeclaration(Declaration* decl) {
    FunctionGuard g(std::cout, "ExitDeclaration");
    if (type_stack_.size() == 0) {
      context_.logger.warning(
          "Unexpected: empty type stack on exiting declaration");
    } else {
      type_stack_.erase(type_stack_.end() - 1);
    }
  }

  bool VisitDeclarationSpecifiers(DeclarationSpecifiers* decl) {
    FunctionGuard g(std::cout, "VisitDeclarationSpecifiers");
    // = storage_class_spec... type_spec... type_quantifier... function_spec...
    if (type_finished_) {
      type_stack_.push_back(nullptr);
      type_finished_ = false;
    }
    if (decl->childs->type == FrontEnv::Tag("type_specifier")) {
      auto& type = reinterpret_cast<Terminal*>(
          reinterpret_cast<NonTerminal*>(decl->childs)->childs)->value;
      type_stack_.back() = type_checker_.FromString(type);
      if (type_stack_.back() == nullptr) {
        context_.logger.error(std::string("Unresolved: type ") + type);
      }
    } else {
      context_.logger.error("Unimplemented: declaration-specifiers");
    }
    return true;
  }
  void ExitDeclarationSpecifiers(DeclarationSpecifiers* decl) {
    FunctionGuard g(std::cout, "ExitDeclarationSpecifiers");
    if (!decl->childs->right) { // type is complete
      type_finished_ = true;
    }
  }

  bool VisitInitDeclarator(InitDeclarator* decl) {
    // = init_declarator , init_declarator
    // = declarator
    // = declarator = initializer
    return true;
  }
  void ExitInitDeclarator(InitDeclarator* decl) {
    FunctionGuard g(std::cout, "ExitInitDeclarator");
    if (decl->childs->type != FrontEnv::Tag("declarator")) {
      return;
    }
    // do initializaton here
    syntax::Identifier identifier = identifier_stack_.back();
    identifier_stack_.pop_back();
    if (decl->childs->right && decl->childs->right->type == '=') {
      // initialization
      llvm::Value* v = value_stack_.back();
      value_stack_.pop_back();
      v = type_checker_.CastTo(v, identifier.type->llvm);
      context_.builder.CreateStore(v, identifier.value);
    }
    context_stack_.back().identifier.Insert(identifier);
  }

  bool VisitDeclarator(Declarator* decl) {
    FunctionGuard g(std::cout, "VisitDeclarator");
    // = pointer direct_declarator
    // = direct_declarator
    if (decl->childs->right) {
      // has pointer spec
      context_.logger.warning("Unimplemented: pointer");
    }
    return true;
  }

  bool VisitDirectDeclarator(DirectDeclarator* decl) {
    FunctionGuard g(std::cout, "VisitDirectDeclarator");
    // = IDENTIFIER
    // = direct_declarator [ ... ]
    // = direct_declarator ( ... )
    if (decl->childs->right) {
      context_.logger.warning("Unimplemented: array");
    } else {
      if (type_stack_.size() == 0) {
        context_.logger.error(
            "Unexpected: empty type stack in direct-declarator");
      } else {
        std::cout << __LINE__ << std::endl;
        Terminal* p = reinterpret_cast<Terminal*>(decl->childs);
        assert(type_stack_.back()->llvm != nullptr);
        std::cout << type_stack_.back()->ToString() << std::endl;
        std::cout << __LINE__ << std::endl;
        llvm::Value* v =
            context_.builder.CreateAlloca(type_stack_.back()->llvm);
        std::cout << __LINE__ << std::endl;
        identifier_stack_.push_back(
            syntax::Identifier(p->value, type_stack_.back(), v));
        std::cout << __LINE__ << std::endl;
      }
    }
    return true;
  }

  bool VisitInitializer(Initializer* init) {
    // = assignment_expression
    // = { ... (,)}
    return true;
  }

  /* Control */

  Status status() const override { return status_; }
  syntax::Context& context() { return context_; }
  const syntax::Context& context() const { return context_; }
 protected:
  Status status_;
  syntax::Context context_;
  syntax::TypeChecker type_checker_;
  // working area
  bool type_finished_ = true;
  std::vector<syntax::InternalTypePtr> type_stack_;
  std::vector<llvm::Value*> value_stack_;
  std::vector<syntax::Identifier> identifier_stack_;
  // construct area
  // match rule: if no exact match, choose unique castable,
  // or raise an ambiguity error.
  std::vector<syntax::StackableContext> context_stack_;
};

}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_VISITOR_H_