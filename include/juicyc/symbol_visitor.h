#ifndef JUICYC_SYMBOL_VISITOR_H_
#define JUICYC_SYMBOL_VISITOR_H_

#include "status.h"
#include "util/util.h"
#include "env.h"

namespace juicyc {

#define MAKE_ENTRY(TYPE) virtual bool Visit##TYPE(TYPE*) { return false; } \
	                       virtual void Exit##TYPE(TYPE*) {}

struct Terminal;
struct NonTerminal;

struct AssignmentExpression;
struct BinaryExpression;
struct CompoundStatement;
struct Declaration;
struct DeclarationSpecifiers;
struct Declarator;
struct DirectDeclarator;
struct ExpressionStatement;
struct FunctionDefinition;
struct LabeledStatement;
struct InitDeclarator;
struct Initializer;
struct IterationStatement;
struct JumpStatement;
struct ParameterDeclaration;
struct ParameterList;
struct PostfixExpression;
struct Root;
struct SelectionStatement;
struct TernaryExpression;
struct UnaryExpression;

// factor out important node and implement 
// their builder here.
class SymbolVisitor : public NoCopy {
 public:
 	SymbolVisitor() = default;
 	virtual ~SymbolVisitor() {}
 	// implementation required
  virtual void VisitTerminal(Terminal*) = 0;
  virtual void ExitTerminal(Terminal*) = 0;
  virtual void VisitNonTerminal(NonTerminal*) = 0;
  virtual void ExitNonTerminal(NonTerminal*) = 0;
  // optional
  MAKE_ENTRY(AssignmentExpression)
  MAKE_ENTRY(BinaryExpression)
  MAKE_ENTRY(CompoundStatement)
  MAKE_ENTRY(Declaration)
  MAKE_ENTRY(DeclarationSpecifiers)
  MAKE_ENTRY(Declarator)
  MAKE_ENTRY(DirectDeclarator)
  MAKE_ENTRY(ExpressionStatement)
  MAKE_ENTRY(FunctionDefinition)
  MAKE_ENTRY(LabeledStatement)
  MAKE_ENTRY(InitDeclarator)
  MAKE_ENTRY(Initializer)
  MAKE_ENTRY(IterationStatement)
  MAKE_ENTRY(JumpStatement)
  MAKE_ENTRY(ParameterDeclaration)
  MAKE_ENTRY(ParameterList)
  MAKE_ENTRY(PostfixExpression)
  MAKE_ENTRY(Root)
  MAKE_ENTRY(SelectionStatement)
  MAKE_ENTRY(TernaryExpression)
  MAKE_ENTRY(UnaryExpression)
  virtual Status status() const = 0;
};

#undef MAKE_ENTRY

} // namespace juicyc

#endif // JUICYC_SYMBOL_VISITOR_H_