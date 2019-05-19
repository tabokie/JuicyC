#include "symbol.h"
#include "symbol_visitor.h"

void juicyc::Symbol::Invoke(juicyc::SymbolVisitor& visitor) {
  if (is_terminal) {
    reinterpret_cast<juicyc::Terminal*>(this)->Invoke(visitor);
  } else {
    reinterpret_cast<juicyc::NonTerminal*>(this)->Invoke(visitor);
  }
}