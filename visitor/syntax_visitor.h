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
      case IDENTIFIER: {
        if (variable_state_ == kAccess) {
          auto& p = FindVariable(n->value);
          if (!p) {
            Fatal(std::string("Unresolved variable ") + n->value);
            value_stack_.push_back(nullptr); // placeholder
          } else {
            assert(p->value);
            value_stack_.push_back(p->value);
          }
        }
      }
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
    return true;
  }

  /* Expression */

  // expression -> assignment_expression,...
  bool VisitAssignmentExpression(AssignmentExpression* expr) {
    context_.logger.trace(std::string("VisitAssignmentExpression"));
    // = conditional_expression // aka ternary
    // = unary_expression assignment_operator assignment_expression
    return true;
  }
  void ExitAssignmentExpression(AssignmentExpression* expr) {
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
        context_.builder.CreateAlignedStore(value, identifier_value, 4);
        break;
        case ADD_ASSIGN:
        if (fp) {
          value = context_.builder.CreateFAdd(value, identifier_value);
        } else {
          value = context_.builder.CreateAdd(value, identifier_value);
        }
        context_.builder.CreateAlignedStore(value, identifier_ptr, 4);
        break;
        default:
        Fatal("Unimplemented: assignment");
        break;
      }
    }
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
    if (expr && expr->at(1) && value_stack_.size() >= 2) {
      FunctionGuard l(std::cout, "Calc");
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
        case '*': ret = fp ? context_.builder.CreateFMul(a, b)
                           : context_.builder.CreateMul(a, b);
        break;
        case '/': ret = fp ? context_.builder.CreateFDiv(a, b)
                           : context_.builder.CreateSDiv(a, b);
        break;
        case '%': if (fp) Fatal("Mod operation on floating point");
                  else ret = context_.builder.CreateSRem(a, b);
        break;
        case '+': ret = fp ? context_.builder.CreateFAdd(a, b)
                           : context_.builder.CreateAdd(a, b);
        break;
        case '-': ret = fp ? context_.builder.CreateFSub(a, b)
                           : context_.builder.CreateSub(a, b);
        break;
        case LEFT_OP: if (fp) Fatal("Left shift operation on floating point");
                      else ret = context_.builder.CreateShl(a, b);
        break;
        case RIGHT_OP: if (fp) Fatal("Right shift operation on floating point");
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

  bool VisitPostfixExpression(PostfixExpression* expr) {
    // = primary_expression
    // != postfix [ expression ]
    // = postfix ( )
    // = postfix ( argument_expression_list )
    // != postfix.identifier
    // != postfix->identifier
    // != postfix ++
    // != postfix --
    // != (TYPE) { .., }
    return true;
  }

  /* Function Declaration */

  bool VisitFunctionDefinition(FunctionDefinition* func) {
    FunctionGuard g(std::cout, "VisitFunctionDefinition");
    // != decl_spec declarator declaration... compound_stat
    // = decl_spec declarator compound_stat
    function_state_ = kFunctionDefinition;
    return true;
  }
  void ExitFunctionDefinition(FunctionDefinition* func) {
    assert(context_stack_.size() > 0);
    llvm::Function* f = context_stack_.back().llvm->getParent();
    auto& function = context_stack_.back().hook;
    if (!function->closed) {
      context_.builder.CreateRetVoid();
    }
    assert(function->value == f);
    assert(f != nullptr);
    if (llvm::verifyFunction(*f, &llvm::errs())) {
      status_ = Status::Corruption("verify function");
    }
  }

  bool VisitParameterList(ParameterList* list) {
    FunctionGuard g(std::cout, "VisitParameterList");
    // = parameter_list
    // != parameter_list , `...`
    // = parameter_list , parameter_declaration
    return true;
  }

  bool VisitParameterDeclaration(ParameterDeclaration* decl) {
    FunctionGuard g(std::cout, "VisitParameterDeclaration");
    // = declaration_spec declarator
    // = declaration_spec // declaration not definition
    if (!decl->at(1)) {
      // assert(function_state_ == kFunctionDeclaration);
      // could be void
    } else {
      assert(function_state_ != kNotFunction);
    }
    return true;
  }
  void ExitParameterDeclaration(ParameterDeclaration* decl) {
    FunctionGuard g(std::cout, "ExitParameterDeclaration");
    type_stack_.pop_back();
  }

  bool VisitCompoundStatement(CompoundStatement* stat) {
    if (function_state_ == kNotFunction) {
      // new block
    }
    return true;
  }

  /* Declaration */

  // external declaration other than function
  bool VisitDeclaration(Declaration* decl) {
    FunctionGuard g(std::cout, "VisitDeclaration");
    // = declaration_spec ;
    // = declaration_spec init_declarator ;
    if (!decl->at(1)->is_terminal) {
      // declaration with init_declarator
    } else {
      Fatal("Unimplemented: declaration={declaration_specifiers}");
    }
    return true;
  }
  void ExitDeclaration(Declaration* decl) {
    FunctionGuard g(std::cout, "ExitDeclaration");
    if (type_stack_.size() == 0) {
      context_.logger.warning(
          "Unexpected: empty type stack on exiting declaration");
    } else {
      type_stack_.pop_back();
    }
  }

  // type specification of declaration
  bool VisitDeclarationSpecifiers(DeclarationSpecifiers* decl) {
    FunctionGuard g(std::cout, "VisitDeclarationSpecifiers");
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
      Fatal("Unimplemented: declaration-specifiers");
    }
    return true;
  }
  void ExitDeclarationSpecifiers(DeclarationSpecifiers* decl) {
    FunctionGuard g(std::cout, "ExitDeclarationSpecifiers");
    if (!decl->at(1)) { // type is complete
      is_complete_type_ = true;
    }
  }

  // declare a list of variable
  bool VisitInitDeclarator(InitDeclarator* decl) {
    // = init_declarator , init_declarator
    // = declarator
    // = declarator = initializer
    return true;
  }
  void ExitInitDeclarator(InitDeclarator* decl) {
    FunctionGuard g(std::cout, "ExitInitDeclarator");
    if (decl->at(0)->type != FrontEnv::Tag("declarator")) {
      return;
    }
    // do initializaton here
    syntax::IdentifierPtr identifier =
        std::make_shared<syntax::Identifier>(identifier_stack_.back());
    identifier_stack_.pop_back();
    bool init = (decl->at(1) && decl->at(1)->type == '=');
    llvm::Value* v = nullptr;
    if (init) {
      v = value_stack_.back();
      value_stack_.pop_back();
      v = type_checker_.CastTo(v, identifier->type->llvm);
    } else {
      v = identifier->type->default_value();
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
        global_.identifier.Insert(identifier);
      } else {
        // local
        context_.builder.CreateAlignedStore(v, identifier->value, 4);
        context_stack_.back().identifier.Insert(identifier);
      }
    }
  }

  // declare one variable, possibly a pointer
  bool VisitDeclarator(Declarator* decl) {
    FunctionGuard g(std::cout, "VisitDeclarator");
    // ?= pointer direct_declarator
    // = direct_declarator
    if (decl->at(1)) {
      // has pointer spec
      context_.logger.warning("Unimplemented: pointer");
    }
    return true;
  }

  // declare one variable, could be function, array, struct
  bool VisitDirectDeclarator(DirectDeclarator* decl) {
    FunctionGuard g(std::cout, "VisitDirectDeclarator");
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
    if ((tmp = decl->at(1)) && tmp->type == '(') {
      if (function_state_ == kNotFunction) {
        function_state_ = kFunctionDeclaration;
      }
    } else if (tmp && tmp->type == '[') {
      context_.logger.warning("Unimplemented: array");
    } else {
      variable_state_ = kDefinition;
      if (type_stack_.size() == 0) {
        Fatal("Unexpected: empty type stack in direct-declarator");
      } else {
        // define identifier
        Terminal* p = reinterpret_cast<Terminal*>(decl->at(0));
        assert(type_stack_.back()->llvm != nullptr);
        llvm::Value* v = nullptr;
        if (function_state_ == kNotFunction) {
          if (context_stack_.size() == 0) {
            // global variable
            llvm::GlobalVariable* variable = new llvm::GlobalVariable(
                *(context_.module),
                type_stack_.back()->llvm,
                false, // is constant
                llvm::GlobalValue::ExternalLinkage,
                0,
                p->value);
            variable->setAlignment(4);
            v = variable;
          } else {
            // local variable
            v = context_.builder.CreateAlloca(type_stack_.back()->llvm);
          }
          if (!v) {
            Fatal(std::string("Unexpected: null value when making ") +
                  p->value);
          }
        }
        identifier_stack_.push_back(
            syntax::Identifier(p->value, type_stack_.back(), v));
      }
    }
    return true;
  }
  void ExitDirectDeclarator(DirectDeclarator* decl) {
    FunctionGuard g(std::cout, "ExitDirectDeclarator");
    Symbol* tmp;
    if ((tmp = decl->at(1)) && tmp->type == '(') {
      // status: type[1]=return, identifier[1+k]=function+param
      assert(type_stack_.size() == 1);
      assert(identifier_stack_.size() >= 1);
      if (function_state_ == kNotFunction) {
        Fatal("Unexpected: exiting declarator with parentheses");
      } 
      // initialize function type here
      syntax::InternalTypePtr return_type = type_stack_.front();
      std::vector<llvm::Type*> param_types_rawv;
      std::vector<syntax::InternalTypePtr> param_types_v;
      for (int i = 1; i < identifier_stack_.size(); i++) {
        param_types_v.push_back(identifier_stack_[i].type);
        param_types_rawv.push_back(identifier_stack_[i].type->llvm);
      }
      llvm::ArrayRef<llvm::Type*> param_types(param_types_rawv);
      llvm::FunctionType* type = llvm::FunctionType::get(
          return_type->llvm,
          param_types,
          false);
      llvm::Function* function = llvm::Function::Create(
          type,
          llvm::Function::ExternalLinkage,  // should be private and mangle
          identifier_stack_.front().name,
          context_.module.get());
      auto identifier = std::make_shared<syntax::FunctionIdentifier>(
          identifier_stack_.back().name,
          return_type,
          param_types_v,
          function);
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
        if (identifier_stack_.size() > 1) {
          auto identifier_iter = identifier_stack_.begin() + 1;
          for (auto& arg_iter : function->args()) {
            arg_iter.setName(identifier_iter->name);
            llvm::Value* v = context_.builder.CreateAlloca(identifier_iter->type->llvm);
            context_.builder.CreateAlignedStore(&arg_iter, v, 4);
            identifier_iter->value = v;
            // identifier_iter->is_arg = true;
            identifier_iter++;
          }
        }
      }
      function_state_ = kNotFunction;
      type_stack_.pop_back();
    } else if (!decl->at(1)) {
      variable_state_ = kAccess;
    }
  }

  // initialize a new declared variable
  bool VisitInitializer(Initializer* init) {
    // = assignment_expression
    // = { ... (,)}
    return true;
  }

  /* Control */

  bool VisitExpressionStatement(ExpressionStatement* stat) {
    // ;
    // expression ;
    return true;
  }

  bool VisitLabeledStatement(LabeledStatement* stat) {
    // = IDENTIFIER : statement
    // = CASE constant_expression : statement
    // = DEFAULT : statement
    return true;
  }

  bool VisitSelectionStatement(SelectionStatement* stat) {
    // = if ( expression ) statement
    // = if ( expression ) statement else statement
    // = switch ( expression ) statement
    return true;
  }
  void ExitSelectionStatement(SelectionStatement* stat) {
    if (stat->at(0)->type == IF) {

    }
  }

  bool VisitIterationStatement(IterationStatement* stat) {
    // = while ( expression ) statement
    // = do statement while ( expression ) ;
    // = for ( expression_stat expression_stat ) statement
    // = for ( expression_stat expression_stat expression ) statement
    // = for ( declaration expression_stat ) statement
    // = for ( declaration expression_stat expression ) statement
    return true;
  }

  bool VisitJumpStatement(JumpStatement* stat) {
    // = goto IDENTIFIER ;
    // = continue ;
    // = break ;
    // = return ;
    // = return expression ;
    return true;
  }
  void ExitJumpStatement(JumpStatement* stat) {
    FunctionGuard g(std::cout, "ExitJumpStatement");
    if (stat->at(0)->type == RETURN) {
      std::cout << "RETURN" << std::endl;
      assert(context_stack_.size() > 0);
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
  };
  VariableState variable_state_ = kAccess;
  enum FunctionState {
    kNotFunction = 0,
    kFunctionDeclaration = 1,
    kFunctionDefinition = 2,
  };
  FunctionState function_state_ = kNotFunction;
  std::vector<syntax::InternalTypePtr> type_stack_;
  std::vector<llvm::Value*> value_stack_;
  std::vector<syntax::Identifier> identifier_stack_;
  // construct area
  // match rule: if no exact match, choose unique castable,
  // or raise an ambiguity error.
  syntax::GlobalContext global_;
  std::vector<syntax::StackableContext> context_stack_;
};

}  // namespace juicyc

#endif  // JUICYC_VISITOR_SYNTAX_VISITOR_H_