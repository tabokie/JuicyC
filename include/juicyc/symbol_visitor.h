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
struct UnaryExpression;
struct BinaryExpression;
struct TernaryExpression;

// factor out important node and implement 
// their builder here.
class SymbolVisitor : public NoCopy {
 public:
 	SymbolVisitor() = default;
 	virtual ~SymbolVisitor() {}
 	// voluntary
  virtual void VisitTerminal(Terminal*) = 0;
  virtual void ExitTerminal(Terminal*) = 0;
  virtual void VisitNonTerminal(NonTerminal*) = 0;
  virtual void ExitNonTerminal(NonTerminal*) = 0;
  // default not implemented
  MAKE_ENTRY(UnaryExpression)
  MAKE_ENTRY(BinaryExpression)
  MAKE_ENTRY(TernaryExpression)
  virtual Status status() const = 0;
};

#undef MAKE_ENTRY

} // namespace juicyc

#endif // JUICYC_SYMBOL_VISITOR_H_