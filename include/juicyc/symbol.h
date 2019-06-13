#ifndef JUICYC_SYMBOL_H_
#define JUICYC_SYMBOL_H_

#include "symbol_visitor.h"

#include <string>

namespace juicyc {

struct Symbol {
  bool is_terminal = true;
  int type = 0;
  Symbol* left = nullptr;
  Symbol* right = nullptr;
  Symbol(bool is_terminal): is_terminal(is_terminal) {}
  ~Symbol() {
    delete right;
  }
  virtual void Invoke(SymbolVisitor& visitor) = 0;
  // helper function
  template <class ...Args>
  static void MakeSibling(Symbol* head, Symbol* tail, Args... siblings) {
    head->right = tail;
    tail->left = head;
    MakeSibling(tail, siblings...);
  }
  static void MakeSibling(Symbol* head, Symbol* tail) {
    head->right = tail;
    tail->left = head;
  }
};

struct Terminal : public Symbol {
  // refer to tagger for detailed string
  std::string value;
  uint16_t file;
  uint32_t line;
  uint32_t col;
  Terminal() : Symbol(true) { }
  ~Terminal() { }
  virtual void Invoke(SymbolVisitor& visitor) override {
    visitor.VisitTerminal(this);
    visitor.ExitTerminal(this);
  }
};

struct NonTerminal : public Symbol {
  Symbol* childs;
  NonTerminal() : Symbol(false) { }
  ~NonTerminal() {
    delete childs;
  }
  virtual void Invoke(SymbolVisitor& visitor) override {
    visitor.VisitNonTerminal(this);
    Symbol* cur = childs;
    while (cur) {
      cur->Invoke(visitor);
      cur = cur->right;
    }
    visitor.ExitNonTerminal(this);
  }
  // get n-th child
  Symbol* at(uint32_t n) {
    if (!childs || n == 0) return childs;
    Symbol* cur = childs;
    for (uint32_t i = 0; i < n && cur; i++) {
      cur = cur->right;
    }
    return cur;
  }
};

#define DUPLICATE_SYMBOL(TYPE) \
struct TYPE : public NonTerminal { \
  TYPE() : NonTerminal() {} \
  ~TYPE() {} \
  virtual void Invoke(SymbolVisitor& visitor) override { \
    bool use_base_impl = false; \
    if (!visitor.Visit##TYPE(this)) { \
      use_base_impl = true; \
      visitor.VisitNonTerminal(this); \
    } \
    Symbol* cur = childs; \
    while (cur) { \
      cur->Invoke(visitor); \
      cur = cur->right; \
    } \
    if (use_base_impl) { \
      visitor.ExitNonTerminal(this); \
    } else { \
      visitor.Exit##TYPE(this); \
    } \
  } \
}

#define DERIVE_SYMBOL(TYPE, MEMBER) \
struct TYPE : public NonTerminal { \
  MEMBER \
  TYPE() : NonTerminal() {} \
  ~TYPE() {} \
  virtual void Invoke(SymbolVisitor& visitor) override { \
    bool use_base_impl = false; \
    if (!visitor.Visit##TYPE(this)) { \
      use_base_impl = true; \
      visitor.VisitNonTerminal(this); \
    } \
    Symbol* cur = childs; \
    while (cur) { \
      cur->Invoke(visitor); \
      cur = cur->right; \
    } \
    if (use_base_impl) { \
      visitor.ExitNonTerminal(this); \
    } else { \
      visitor.Exit##TYPE(this); \
    } \
  } \
}

DUPLICATE_SYMBOL(AssignmentExpression);
DUPLICATE_SYMBOL(BinaryExpression);
DUPLICATE_SYMBOL(CompoundStatement);
DUPLICATE_SYMBOL(Declaration);
DUPLICATE_SYMBOL(DeclarationSpecifiers);
DUPLICATE_SYMBOL(Declarator);
DUPLICATE_SYMBOL(DirectDeclarator);
DUPLICATE_SYMBOL(ExpressionStatement);
DUPLICATE_SYMBOL(FunctionDefinition);
DUPLICATE_SYMBOL(LabeledStatement);
DUPLICATE_SYMBOL(InitDeclarator);
DUPLICATE_SYMBOL(Initializer);
DUPLICATE_SYMBOL(IterationStatement);
DUPLICATE_SYMBOL(JumpStatement);
DUPLICATE_SYMBOL(ParameterDeclaration);
DUPLICATE_SYMBOL(ParameterList);
DUPLICATE_SYMBOL(PostfixExpression);
DUPLICATE_SYMBOL(Root);
DUPLICATE_SYMBOL(SelectionStatement);
DUPLICATE_SYMBOL(TernaryExpression);
DUPLICATE_SYMBOL(UnaryExpression);

// CompoundStatement
// ExpressionStatement
// SelectionStatement
// IterationStatement
// JumpStatement
// LabeledStatement

#undef DUPLICATE_SYMBOL
#undef DERIVE_SYMBOL

// TODO: add more internal node type 
// inheriting from NonTerminal.
// Add type identifier.

} // namespace juicyc

#endif // JUICYC_SYMBOL_H_