#ifndef JUICYC_SYMBOL_VISITOR_H_
#define JUICYC_SYMBOL_VISITOR_H_

#include "status.h"
#include "util/util.h"
#include "env.h"

namespace juicyc {

struct Terminal;
struct NonTerminal;
struct Expression;

// factor out important node and implement 
// their builder here.
class SymbolVisitor : public NoCopy {
 public:
 	// voluntary
  virtual bool VisitTerminal(Terminal*) = 0;
  virtual bool ExitTerminal(Terminal*) = 0;
  virtual bool VisitNonTerminal(NonTerminal*) = 0;
  virtual bool ExitNonTerminal(NonTerminal*) = 0;
  // default not implemented
  virtual bool VisitExpression(Expression*) { return false; }
  virtual bool ExitExpression(Expression*) { return false; }
  virtual Status status() const = 0;
};

} // namespace juicyc

#endif // JUICYC_SYMBOL_VISITOR_H_