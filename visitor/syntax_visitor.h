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
#include "frontend/gen_parse.hh"

#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>

#include <iostream>

namespace juicyc {
class SyntaxVisitor : public SymbolVisitor {
 public:
  SyntaxVisitor(Env* env) : SyntaxVisitor(env, "main") {}
  SyntaxVisitor(Env* env, std::string name)
      : context_(name),
        global_(&type_checker_) {
    context_.env = env;
    type_checker_.set_context(&context_);
  };
  ~SyntaxVisitor() {}

  // helper
  void Fatal(std::string message) {
    context_.logger.error(message);
    if (status_.ok()) {
      status_ = Status::Corruption("Fatal error");
    }
  }
  syntax::IdentifierPtr FindVariable(std::string name) {
    syntax::IdentifierPtr ret = nullptr;
    for (int i = context_stack_.size() - 1; i >= 0; i--) {
      if (context_stack_[i].identifier.Get(name, ret).ok()) {
        return ret;
      }
    }
    global_.identifier.Get(name, ret);
    return ret;
  }
  void PopBlock() {
    context_stack_.pop_back();
    if (context_stack_.size() > 0) {
      context_.builder.SetInsertPoint(context_stack_.back().llvm);
    } else {
      context_.builder.ClearInsertionPoint();
    }
  }
  std::string dump() {
    std::string tmp = "[";
    for (auto& i : identifier_stack_) {
      if (i)
        tmp += i->name + ",";
      else
        tmp += "nil,";
    }
    tmp += "]";
    return std::to_string(type_stack_.size()) + "-" +
           std::to_string(value_stack_.size()) + "-" +
           std::to_string(identifier_stack_.size()) + tmp + "-" +
           std::to_string(context_stack_.size()) + ".";
  }
  std::string gen_block_name() {
    static uint32_t id = 0;
    return std::string("tmpb-") + std::to_string(id++);
  }

  void VisitTerminal(Terminal* n) override {
    FunctionGuard g(std::cout, dump() + std::string("VisitTerminal:") + n->value);
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
          n->value.substr(1, n->value.size()-2), "const char*"));
      break;
      case IDENTIFIER: {
        if (variable_state_ == kAccess ||
            variable_state_ == kAddress) {
          auto& p = FindVariable(n->value);
          if (!p) {
            Fatal(std::string("Unresolved variable ") + n->value);
            value_stack_.push_back(nullptr); // placeholder
          } else {
            assert(p->value);
            llvm::Value* v = p->value;
            if (variable_state_ == kAccess) {
              v = context_.builder.CreateAlignedLoad(
                  v->getType()->getContainedType(0),
                  v, 4);
            }
            value_stack_.push_back(v);
          }
        } else {
          // function or definition state
          identifier_stack_.push_back(std::make_shared<syntax::Identifier>(
              n->value, nullptr, nullptr));
        }
        variable_state_ = kAccess;
      }
      break;
      default:
      break;
    }
  }
  void ExitTerminal(Terminal* n) override {}

  void VisitNonTerminal(NonTerminal* n) override {
    context_.logger.trace(std::string("VisitNonTerminal:") +
        FrontEnv::Untag(n->type));
  }
  void ExitNonTerminal(NonTerminal* n) override {}

  bool VisitRoot(Root* root) override {
    context_.logger.trace(std::string("VisitRoot"));
    return true;
  }

  /* Expression */

  // expression -> assignment_expression,...
  bool VisitAssignmentExpression(AssignmentExpression* expr) override {
    context_.logger.trace(std::string("VisitAssignmentExpression"));
    // = conditional_expression // aka ternary
    // = unary_expression assignment_operator assignment_expression
    if (expr->at(1)) {
      variable_state_ = kAddress;
    }
    return true;
  }
  void ExitAssignmentExpression(AssignmentExpression* expr) override {
    if (expr->at(1)) {
      // assignment here
      Symbol* terminal = dynamic_cast<NonTerminal*>(expr->at(1))->at(0);
      assert(terminal);
      assert(value_stack_.size() >= 2);
      llvm::Value* value = value_stack_.back();
      if (value->getType()->isPointerTy()) {
        value = context_.builder.CreateAlignedLoad(value->getType()->getContainedType(0),
                                                   value, 4);
      }
      value_stack_.pop_back();
      llvm::Value* identifier_ptr = value_stack_.back();
      llvm::Value* identifier_value =
          context_.builder.CreateAlignedLoad(identifier_ptr->getType()->getContainedType(0),
                                             identifier_ptr, 4);
      value_stack_.pop_back();
      bool fp = 
          (value->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           value->getType()->getTypeID() == llvm::Type::FloatTyID ||
           identifier_value->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           identifier_value->getType()->getTypeID() == llvm::Type::FloatTyID);
      switch (terminal->type) {
        case '=':
        value = type_checker_.CastTo(value, identifier_value->getType(), false);
        if (!value) {
          Fatal("Failed to implicit cast type");
        }
        context_.builder.CreateAlignedStore(value, identifier_ptr, 4);
        break;
        case ADD_ASSIGN:
        if (fp) {
          value = context_.builder.CreateFAdd(value, identifier_value);
        } else {
          value = context_.builder.CreateAdd(value, identifier_value);
        }
        value = type_checker_.CastTo(value, identifier_value->getType(), false);
        if (!value) {
          Fatal("Failed to implicit cast type");
        }
        context_.builder.CreateAlignedStore(value, identifier_ptr, 4);
        break;
        default:
        Fatal("Unimplemented: assignment");
        break;
      }
    }
  }

  bool VisitTernaryExpression(TernaryExpression* expr) override {
    context_.logger.trace(std::string("VisitTernaryExpression"));
    // = binary_expression
    // = binary ? expression : ternary_expression
    return true;
  }

  bool VisitBinaryExpression(BinaryExpression* expr) override {
    context_.logger.trace(std::string("VisitBinaryExpression"));
    // = binary_expression OP cast_expression
    return true;
  }
  void ExitBinaryExpression(BinaryExpression* expr) override {
    FunctionGuard g(std::cout, dump() + "ExitBinaryExpression");
    if (expr && expr->at(1) && value_stack_.size() >= 2) {
      FunctionGuard l(std::cout, dump() + "Calc");
      Symbol* op = expr->at(1);
      llvm::Value* b = value_stack_.back();
      value_stack_.pop_back();
      llvm::Value* a = value_stack_.back();
      value_stack_.pop_back();
      bool fp = 
          (a->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           a->getType()->getTypeID() == llvm::Type::FloatTyID ||
           b->getType()->getTypeID() == llvm::Type::DoubleTyID ||
           b->getType()->getTypeID() == llvm::Type::FloatTyID);
      llvm::Value* ret = a;  // wrong but usable
      switch (op->type) {
        case '*': ret = fp ? ret = context_.builder.CreateFMul(a, b)
                           : ret = context_.builder.CreateMul(a, b);
        break;
        case '/': ret = fp ? ret = context_.builder.CreateFDiv(a, b)
                           : ret = context_.builder.CreateSDiv(a, b);
        break;
        case '%': if (fp) Fatal("Mod operation on floating point");
                  else ret = context_.builder.CreateSRem(a, b);
        break;
        case '+': ret = fp ? ret = context_.builder.CreateFAdd(a, b)
                           : ret = context_.builder.CreateAdd(a, b);
        break;
        case '-': ret = fp ? ret = context_.builder.CreateFSub(a, b)
                           : ret = context_.builder.CreateSub(a, b);
        break;
        case LEFT_OP: if (fp) Fatal("Left shift operation on floating point");
                      else ret = context_.builder.CreateShl(a, b);
        break;
        case RIGHT_OP: if (fp) Fatal("Right shift operation on floating point");
                       else ret = context_.builder.CreateAShr(a, b);
        break;
        case '<': fp ? ret = context_.builder.CreateFCmpULT(a, b)
                     : ret = context_.builder.CreateICmpULT(a, b);
        break;
        case '>': fp ? ret = context_.builder.CreateFCmpOGT(a, b)
                     : ret = context_.builder.CreateICmpSGT(a, b);
        break;
        case LE_OP: fp ? ret = context_.builder.CreateFCmpOLE(a, b)
                       : ret = context_.builder.CreateICmpSLE(a, b);
        break;
        case GE_OP: fp ? ret = context_.builder.CreateFCmpOGE(a, b)
                       : ret = context_.builder.CreateICmpSGE(a, b);
        break;
        case EQ_OP: fp ? ret = context_.builder.CreateFCmpOEQ(a, b)
                       : ret = context_.builder.CreateICmpEQ(a, b);
        break;
        case NE_OP: fp ? ret = context_.builder.CreateFCmpONE(a, b)
                       : ret = context_.builder.CreateICmpNE(a, b);
        break;
        case '&': if (fp) Fatal("And operation on floating point");
                  else ret = context_.builder.CreateAnd(a, b);
        break;
        case '^': if (fp) Fatal("Xor operation on floating point");
                  else ret = context_.builder.CreateXor(a, b);
        break;
        case '|': if (fp) Fatal("Or operation on floating point");
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

  bool VisitUnaryExpression(UnaryExpression* expr) override {
    context_.logger.trace(std::string("VisitUnaryExpression"));
    // = postfix_expression
    // = OP unary_expression/cast_expression
    // = SIZEOF ( TYPE )
    return true;
  }
  void ExitUnaryExpression(UnaryExpression* expr) override {
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

  int value_top;

  bool VisitPostfixExpression(PostfixExpression* expr) override {
    // = primary_expression
    // != postfix [ expression ]
    // = postfix ( )
    // = postfix ( argument_expression_list )
    // != postfix.identifier
    // != postfix->identifier
    // != postfix ++
    // != postfix --
    // != (TYPE) { .., }
    if (expr->at(1) && expr->at(1)->type == '(') {
      variable_state_ = kFunction;
      value_top = value_stack_.size();
    }
    return true;
  }
  void ExitPostfixExpression(PostfixExpression* expr) override {
    if (expr->at(1) && expr->at(1)->type) {
      // create function call
      assert(identifier_stack_.size() > 0);
      std::string name = identifier_stack_.back()->name;
      identifier_stack_.pop_back();
      syntax::FunctionIdentifierPtr function = nullptr;
      std::vector<syntax::InternalTypePtr> args;
      assert(value_stack_.size() >= value_top);
      for (int i = value_top; i < value_stack_.size(); i++)
        args.push_back(type_checker_.FromRaw(value_stack_[i]->getType()));
      for (int i = context_stack_.size() - 1; i >= 0; i --) {
        auto& context = context_stack_[i];
        if (i != context_stack_.size() - 1 && context.sticky)
          continue;
        if (context.function.Get(name, args, function).ok()) break;
      }
      if (!function) {
        global_.function.Get(name, args, function);
      }
      if (!function) {
        Fatal(std::string("Unexpected: unresolved function identifier ") +
              name);
        value_stack_.erase(value_stack_.begin() + value_top, value_stack_.end());
      } else {
        assert(function->value->arg_size() == args.size() || function->varargs);
        for (int i = value_top; i < value_stack_.size(); i++) {
          value_stack_[i] = type_checker_.CastTo(value_stack_[i], function->args[i-value_top]->llvm, false);
          assert(value_stack_[i]);
        }
        std::vector<llvm::Value*> needed_value(value_stack_.begin() + value_top, value_stack_.end());
        llvm::Value* ret = context_.builder.CreateCall(function->value, needed_value, "calltmp");
        value_stack_.erase(value_stack_.begin() + value_top, value_stack_.end());
        value_stack_.push_back(ret);
      }
    }
  }

  /* Function Declaration */

  bool VisitFunctionDefinition(FunctionDefinition* func) override {
    FunctionGuard g(std::cout, dump() + "VisitFunctionDefinition");
    // != decl_spec declarator declaration... compound_stat
    // = decl_spec declarator compound_stat
    function_state_ = kFunctionDefinition;
    sticky_context_ = true;
    return true;
  }
  void ExitFunctionDefinition(FunctionDefinition* func) override {
    FunctionGuard g(std::cout, dump() + "ExitFunctionDefinition");
    assert(context_stack_.size() > 0);
    llvm::Function* f = context_stack_.back().llvm->getParent();
    auto& function = context_stack_.back().hook;
    if (!function->closed) {
      if (function->ret && !function->ret->llvm->isVoidTy()) {
        context_.builder.CreateRet(function->ret->default_value());
      } else {
        context_.builder.CreateRetVoid();
      }
    }
    assert(function->value == f);
    assert(f != nullptr);
    if (llvm::verifyFunction(*f, &llvm::errs())) {
      status_ = Status::Corruption("verify function");
    }
    PopBlock();
    sticky_context_ = false;
  }

  bool VisitParameterList(ParameterList* list) override {
    FunctionGuard g(std::cout, dump() + "VisitParameterList");
    // = parameter_list
    // = parameter_list , `...`
    // = parameter_list , parameter_declaration
    return true;
  }
  void ExitParameterList(ParameterList* list) override {
    FunctionGuard g(std::cout, dump() + "ExitParameterList");
    if (list->at(1) && list->at(1)->type == ELLIPSIS) {
      identifier_stack_.push_back(nullptr);  // as placeholder
    }
  }

  bool VisitParameterDeclaration(ParameterDeclaration* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitParameterDeclaration");
    // = declaration_spec declarator
    // = declaration_spec // declaration not definition
    // = declaration_spec abstract_declarator
    if (!decl->at(1)) {
      // assert(function_state_ == kFunctionDeclaration);
      // could be void
    } else {
      assert(function_state_ != kNotFunction);
    }
    return true;
  }
  void ExitParameterDeclaration(ParameterDeclaration* decl) override {
    FunctionGuard g(std::cout, dump() + "ExitParameterDeclaration");
    if (!decl->at(1)) {  // declaration not definition
      identifier_stack_.push_back(std::make_shared<syntax::Identifier>(
          "", type_stack_.back(), nullptr));
    } else if (decl->at(1)->type == FrontEnv::Tag("abstract_declarator")) {
      // faulty assumption
      identifier_stack_.push_back(std::make_shared<syntax::Identifier>(
          "", type_stack_.back()->pointer(), nullptr));
    } else {
      identifier_stack_.back()->type = type_stack_.back();
    }
    type_stack_.pop_back();
  }

  bool VisitCompoundStatement(CompoundStatement* stat) override {
    FunctionGuard g(std::cout, dump() + "VisitCompoundStatement");
    if (function_state_ == kNotFunction) {
      // new block
      assert(context_stack_.size() != 0);
      context_stack_.push_back(syntax::StackableContext(&type_checker_));
      if (sticky_context_) {
        llvm::BasicBlock* block = llvm::BasicBlock::Create(
            context_.llvm, gen_block_name());
        context_.builder.SetInsertPoint(block);
        context_stack_.back().llvm = block;
      } else {
        syntax::FunctionIdentifierPtr pf = nullptr;
        for (int i = context_stack_.size() - 1; i >= 0; i ++) {
          if (context_stack_[i].hook) {
            pf = context_stack_[i].hook;
            break;
          }
        }
        assert(pf);
        llvm::BasicBlock* block = llvm::BasicBlock::Create(
            context_.llvm, gen_block_name(),
            pf->value);
        context_.builder.SetInsertPoint(block);
        context_stack_.back().llvm = block;
        context_stack_.back().hook = pf;  
      }

      if (sticky_context_) {
        context_stack_.back().sticky = true;
      }
      sticky_context_ = false;
    } else {
      if (sticky_context_) {
        context_stack_.back().sticky = true;
      }
      function_state_ = kNotFunction;  // reset function state
    }
    return true;
  }
  void ExitCompoundStatement(CompoundStatement* stat) override {
    FunctionGuard g(std::cout, dump() + "ExitCompoundStatement");
    if (!context_stack_.back().sticky) {
      PopBlock();
    } else {
      sticky_context_ = true;
    }
  }

  /* Declaration */

  // external declaration other than function
  bool VisitDeclaration(Declaration* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitDeclaration");
    // = declaration_spec ;
    // = declaration_spec init_declarator ;
    if (!decl->at(1)->is_terminal) {
      // declaration with init_declarator
    } else {
      Fatal("Unimplemented: declaration={declaration_specifiers}");
    }
    return true;
  }
  void ExitDeclaration(Declaration* decl) override {
    FunctionGuard g(std::cout, dump() + "ExitDeclaration");
    if (type_stack_.size() == 0) {
      context_.logger.warning(
          "Unexpected: empty type stack on exiting declaration");
    } else {
      type_stack_.pop_back();
    }
  }

  // type specification of declaration
  bool VisitDeclarationSpecifiers(DeclarationSpecifiers* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitDeclarationSpecifiers");
    // = storage_class_spec... type_spec... type_quantifier... function_spec...
    if (is_complete_type_) {
      type_stack_.push_back(nullptr);
      is_complete_type_ = false;
    }
    if (decl->at(0)->type == FrontEnv::Tag("type_specifier")) {
      auto& type = reinterpret_cast<Terminal*>(
          reinterpret_cast<NonTerminal*>(decl->at(0))->at(0))->value;
      type_stack_.back() = type_checker_.FromString(type);
      if (type_stack_.back() == nullptr) {
        Fatal(std::string("Unresolved: type ") + type);
      }
    } else {
      context_.logger.warning("Unimplemented: declaration-specifiers");
    }
    return true;
  }
  void ExitDeclarationSpecifiers(DeclarationSpecifiers* decl) override {
    FunctionGuard g(std::cout, dump() + "ExitDeclarationSpecifiers");
    if (!decl->at(1)) { // type is complete
      is_complete_type_ = true;
    }
  }

  // declare a list of variable
  bool VisitInitDeclarator(InitDeclarator* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitInitDeclarator");
    // = init_declarator , init_declarator
    // = init_declarator
    // = declarator
    // = declarator = initializer
    return true;
  }
  // initialize identifier:
  // consume one identifier
  void ExitInitDeclarator(InitDeclarator* decl) override {
    FunctionGuard g(std::cout, dump() + "ExitInitDeclarator");
    if (decl->at(1) && decl->at(1)->type == ',' ||
        decl->at(0)->type == FrontEnv::Tag("init_declarator")) {
      return;
    }
    if (function_state_ != kNotFunction) {
      if (function_state_ == kFunctionDeclaration) {
        function_state_ = kNotFunction;
      }
      return;
    }
    /*
    if (!decl->at(1) || decl->at(1)->type != '=') {
      if (decl->at(0)->type == FrontEnv::Tag("declarator") &&
          identifier_stack_.size() > 0){
        // could be function definition skipping init declarator
        identifier_stack_.pop_back();
      }
      return;
    }
    */
    // do initializaton here
    assert(identifier_stack_.size() > 0);
    syntax::IdentifierPtr identifier = identifier_stack_.back();
    identifier_stack_.pop_back();
    bool init = (decl->at(1) && decl->at(1)->type == '=');
    llvm::Value* v = nullptr;
    llvm::Type* inner_type = identifier->type->llvm;
    // if (inner_type->isPointerTy()) inner_type = inner_type->getContainedType(0);
    if (init) {
      v = value_stack_.back();
      value_stack_.pop_back();
      v = type_checker_.CastTo(v, inner_type);
    } else {
      syntax::InternalType tmp(inner_type);
      v = tmp.default_value();
    }
    if (v) {  // null means no need for init
      if (context_stack_.size() == 0) {
        // global
        auto pv = llvm::dyn_cast<llvm::GlobalVariable>(identifier->value);
        if (!pv) {
          Fatal("Identifier not of type GlobalVariable");
        }
        auto pi = llvm::dyn_cast<llvm::Constant>(v);
        if (!pi) {
          Fatal("Global initializer not of type Constant");
        }
        pv->setInitializer(pi);
      } else {
        // local
        context_.builder.CreateAlignedStore(v, identifier->value, 4);
      }
    }
    if (context_stack_.size() == 0) {
      global_.identifier.Insert(identifier);
    } else {
      context_stack_.back().identifier.Insert(identifier);
    }
  }

  // declare one variable, possibly a pointer
  // should push one identifier to stack
  bool VisitDeclarator(Declarator* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitDeclarator");
    // = pointer direct_declarator
    // = direct_declarator
    if (decl->at(1)) {
      // has pointer spec
      // context_.logger.warning("Unimplemented: pointer");
      is_pointer_ = true;
    }
    return true;
  }

  // declare one variable, could be function, array, struct
  bool VisitDirectDeclarator(DirectDeclarator* decl) override {
    FunctionGuard g(std::cout, dump() + "VisitDirectDeclarator");
    // = IDENTIFIER
    // ?= direct_declarator [ ... ]
    //   ... = type_qualifier_list assignment_expr
    //   ... = type_qualifier_list
    //   ... = `static` type_qualifier_list assignment_expr
    //   ... = type_qualifier_list `static` assignment_expr
    //   ... = type_qualifier_list *
    //   ... = *
    // = direct_declarator ( ... )
    //   ... = parameter_type_list
    //   ... != identifier_list
    Symbol* tmp;
    variable_state_ = kDefinition;
    if ((tmp = decl->at(1)) && tmp->type == '(') {
      if (function_state_ == kNotFunction) {
        function_state_ = kFunctionDeclaration;
      }
      if (is_pointer_) {
        type_stack_.back() = type_stack_.back()->pointer();
        is_pointer_ = false;
      }
    } else if (tmp && tmp->type == '[') {
      context_.logger.warning("Unimplemented: array");
    }
    return true;
  }
  // declare variable in VISIT:
  // read 1 type
  // modify 1 identifier
  // declare function in EXIT:
  // consume 1 type (return)
  // consume 1+k identifiers
  void ExitDirectDeclarator(DirectDeclarator* decl) override {
    FunctionGuard g(std::cout, dump() + "ExitDirectDeclarator");
    Symbol* tmp;
    if ((tmp = decl->at(1)) && tmp->type == '(') {
      assert(type_stack_.size() == 1);
      assert(identifier_stack_.size() >= 1);
      if (function_state_ == kNotFunction) {
        Fatal("Unexpected: exiting declarator with parentheses");
      }
      // initialize function type here
      syntax::InternalTypePtr return_type = type_stack_.front();
      std::vector<llvm::Type*> param_types_rawv;
      std::vector<syntax::InternalTypePtr> param_types_v;
      bool varargs = (identifier_stack_.back() == nullptr);
      if (varargs) identifier_stack_.pop_back();
      bool void_input = false;
      for (int i = 1; i < identifier_stack_.size(); i++) {
        assert(identifier_stack_[i]);
        if (identifier_stack_[i]->type->llvm->getTypeID() == llvm::Type::VoidTyID) {
          if (identifier_stack_.size() != 2) {
            Fatal("Multiple parameters with void in them.");
          } else {
            void_input = true;
            break;  // ignore void input
          }
        }
        assert(identifier_stack_[i]->type);
        assert(identifier_stack_[i]->type->llvm);
        param_types_v.push_back(identifier_stack_[i]->type);
        param_types_rawv.push_back(identifier_stack_[i]->type->llvm);
      }
      llvm::ArrayRef<llvm::Type*> param_types(param_types_rawv);
      llvm::FunctionType* type = llvm::FunctionType::get(
          return_type->llvm,
          param_types,
          varargs);
      llvm::Function* function = llvm::Function::Create(
          type,
          llvm::Function::ExternalLinkage,  // should be private and mangle
          identifier_stack_.front()->name,
          context_.module.get());
      auto identifier = std::make_shared<syntax::FunctionIdentifier>(
          identifier_stack_.front()->name,
          return_type,
          param_types_v,
          function);
      identifier->varargs = varargs;
      if (context_stack_.size() == 0) {
        global_.function.Insert(identifier);
      } else {
        context_stack_.back().function.Insert(identifier);
      }
      if (function_state_ == kFunctionDefinition) {
        // enter new block
        context_stack_.push_back(syntax::StackableContext(&type_checker_));
        llvm::BasicBlock* block = llvm::BasicBlock::Create(
            context_.llvm, "entry",
            function);
        context_.builder.SetInsertPoint(block);
        context_stack_.back().llvm = block;
        context_stack_.back().hook = identifier;
        // assign identifier
        if (!void_input && identifier_stack_.size() > 1) {
          auto identifier_iter = identifier_stack_.begin() + 1;
          for (auto& arg_iter : function->args()) {
            auto& deref_iter = *identifier_iter;
            // arg_iter.setName(deref_iter->name);
            llvm::Value* v = context_.builder.CreateAlloca(deref_iter->type->llvm);
            context_.builder.CreateAlignedStore(&arg_iter, v, 4);
            // deref_iter->value = &arg_iter;
            deref_iter->value = v;
            // deref_iter->is_arg = true;
            context_stack_.back().identifier.Insert(deref_iter);
            identifier_iter++;
          }
        }
      }
      identifier_stack_.clear();
      type_stack_.pop_back();
    } else if (!decl->at(1)) {
      if (type_stack_.size() == 0) {
        Fatal("Unexpected: empty type stack in direct-declarator");
      } else if (identifier_stack_.size() == 0) {
        Fatal("Unexpected: empty identifier stack in direct-declarator");
      } else {
        // modify identifier
        assert(type_stack_.back()->llvm != nullptr);
        llvm::Value* v = nullptr;
        syntax::InternalTypePtr type = type_stack_.back();
        if (is_pointer_) {
          type = type->pointer();
          is_pointer_ = false;
        }
        if (function_state_ == kNotFunction) {
          if (context_stack_.size() == 0) {
            // global variable
            llvm::GlobalVariable* variable = new llvm::GlobalVariable(
                *(context_.module),
                type->llvm,
                false, // is constant
                llvm::GlobalValue::ExternalLinkage,
                0,
                identifier_stack_.back()->name);
            variable->setAlignment(4);
            v = variable;
          } else {
            // local variable
            v = context_.builder.CreateAlloca(type->llvm);
          }
          if (!v) {
            Fatal(std::string("Unexpected: null value when making ") +
                  identifier_stack_.back()->name);
          }  
        }
        identifier_stack_.back()->type = type;
        identifier_stack_.back()->value = v;
      }
    }
  }

  // initialize a new declared variable
  bool VisitInitializer(Initializer* init) override {
    // = assignment_expression
    // = { ... (,)}
    return true;
  }

  /* Control */

  bool VisitExpressionStatement(ExpressionStatement* stat) override {
    // ;
    // expression ;
    return true;
  }

  bool VisitLabeledStatement(LabeledStatement* stat) override {
    // = IDENTIFIER : statement
    // = CASE constant_expression : statement
    // = DEFAULT : statement
    return true;
  }

  bool VisitSelectionStatement(SelectionStatement* stat) override {
    // = if ( expression ) statement
    // = if ( expression ) statement else statement
    // = switch ( expression ) statement
    sticky_context_ = true;
    return true;
  }
  void ExitSelectionStatement(SelectionStatement* stat) override {
    if (stat->at(0)->type == IF) {
      assert(context_stack_.size() > 1);
      assert(context_stack_.back().sticky);
      llvm::BasicBlock* else_block = nullptr;
      llvm::BasicBlock* if_block = nullptr;
      llvm::BasicBlock* resume = llvm::BasicBlock::Create(context_.llvm, "resume");
      if (stat->at(5) && stat->at(5)->type == ELSE) {
        assert(context_stack_.size() > 2);
        assert(context_stack_.back().sticky);
        else_block = context_stack_.back().llvm;
        context_stack_.pop_back();
        assert(context_stack_.back().sticky);
        if_block = context_stack_.back().llvm;
        context_stack_.pop_back();
      } else {
        if_block = context_stack_.back().llvm;
        context_stack_.pop_back();
      }

      context_.builder.SetInsertPoint(context_stack_.back().llvm);
      assert(value_stack_.size() > 0);
      assert(value_stack_.back());
      llvm::Value* condv = value_stack_.back();
      // llvm::Value* condv = type_checker_.CastToBoolean(value_stack_.back());
      assert(condv);
      value_stack_.pop_back();

      syntax::FunctionIdentifierPtr pf = nullptr;
      for (int i = context_stack_.size() - 1; i >= 0; i --) {
        if (context_stack_[i].hook) {
          pf = context_stack_[i].hook;
          break;
        }
      }
      assert(pf);

      if (else_block) {
        context_.builder.CreateCondBr(condv, if_block, else_block);
      } else {
        context_.builder.CreateCondBr(condv, if_block, resume);
      }
      if (!if_block->getTerminator()) {
        context_.builder.SetInsertPoint(if_block);
        context_.builder.CreateBr(resume);
      }
      if_block->insertInto(pf->value);
      if (else_block && !else_block->getTerminator()) {
        context_.builder.SetInsertPoint(else_block);
        context_.builder.CreateBr(resume);
      }
      if (else_block)
        else_block->insertInto(pf->value);

      resume->insertInto(pf->value);
      // pf->value->getBasicBlockList().push_back(resume);
      context_.builder.SetInsertPoint(resume);
      context_stack_.back().llvm = resume;
      sticky_context_ = false;    
    } else {
      Fatal("Unimplemented: selection statement");
    }
  }

  bool VisitIterationStatement(IterationStatement* stat) override {
    // = while ( expression ) statement
    // = do statement while ( expression ) ;
    // = for ( expression_stat expression_stat ) statement
    // = for ( expression_stat expression_stat expression ) statement
    // = for ( declaration expression_stat ) statement
    // = for ( declaration expression_stat expression ) statement
    return true;
  }

  bool VisitJumpStatement(JumpStatement* stat) override {
    // = goto IDENTIFIER ;
    // = continue ;
    // = break ;
    // = return ;
    // = return expression ;
    return true;
  }
  void ExitJumpStatement(JumpStatement* stat) override {
    FunctionGuard g(std::cout, dump() + "ExitJumpStatement");
    if (stat->at(0)->type == RETURN) {
      assert(context_stack_.size() > 0);
	  if (context_stack_.back().hook)
	    context_stack_.back().hook->closed = true;
      if (stat->at(1)) {
        assert(value_stack_.size() > 0);
        context_.builder.CreateRet(value_stack_.back());
        value_stack_.pop_back();
      } else {
        context_.builder.CreateRetVoid();
      }
    }
  }

  Status status() const override { return status_; }
  syntax::Context& context() { return context_; }
  const syntax::Context& context() const { return context_; }
 protected:
  Status status_;
  syntax::Context context_;
  syntax::TypeChecker type_checker_;
  // working area
  // top of type stack complete?
  // set and clear in declaration-specifier
  bool is_complete_type_ = true;
  enum VariableState {
    kAccess = 0,
    kDefinition = 1,
    kFunction = 2,
    kAddress = 3,
  };
  VariableState variable_state_ = kAccess;
  enum FunctionState {
    kNotFunction = 0,
    kFunctionDeclaration = 1,
    kFunctionDefinition = 2,
  };
  FunctionState function_state_ = kNotFunction;
  bool is_pointer_ = false;
  std::vector<syntax::InternalTypePtr> type_stack_;
  std::vector<llvm::Value*> value_stack_;
  std::vector<syntax::IdentifierPtr> identifier_stack_;
  // construct area
  // match rule: if no exact match, choose unique castable,
  // or raise an ambiguity error.
  syntax::GlobalContext global_;
  bool sticky_context_;
  std::vector<syntax::StackableContext> context_stack_;
};

}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_VISITOR_H_