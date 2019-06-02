#ifndef JUICYC_SYMBOL_VISITOR_H_
#define JUICYC_SYMBOL_VISITOR_H_

#include "status.h"
#include "util/util.h"
#include "env.h"

namespace juicyc {

struct Terminal;
struct NonTerminal;

// factor out important node and implement 
// their builder here.
class SymbolVisitor : public NoCopy {
 public:
  virtual void VisitTerminal(Terminal*) = 0;
  virtual void ExitTerminal(Terminal*) = 0;
  virtual void VisitNonTerminal(NonTerminal*) = 0;
  virtual void ExitNonTerminal(NonTerminal*) = 0;
  virtual Status status() const = 0;
};

} // namespace juicyc

#endif // JUICYC_SYMBOL_VISITOR_H_