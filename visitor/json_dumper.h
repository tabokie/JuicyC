#ifndef JUICYC_JSON_DUMPER_H_
#define JUICYC_JSON_DUMPER_H_

#include "juicyc/env.h"
#include "juicyc/symbol_visitor.h"
#include "juicyc/symbol.h"

#include <string>

namespace juicyc {

class JsonDumper : public SymbolVisitor {
 public:
  JsonDumper(std::string output, Env* env)
      : output_(output),
        env_(env) { Init(); }
  ~JsonDumper() { env_->output_system()->fclose(os_); }
  void VisitTerminal(Terminal* n) override {
    (*os_) << std::string(indent_, ' ')
           << static_cast<int>(n->type)
           << " (" << (n->value) << "),\n";
  }
  void ExitTerminal(Terminal* n) override {}
  void VisitNonTerminal(NonTerminal* n) override {
    (*os_) << std::string(indent_, ' ')
           << FrontEnv::Untag(n->type) << ": {\n";
    indent_ += 2;
  }
  void ExitNonTerminal(NonTerminal* n) override {
    indent_ -= 2;
    (*os_) <<  std::string(indent_, ' ') <<  "},\n";
  }
  Status status() const override {
    return status_;
  }
 protected:
  std::string output_;
  Env* env_;
  OStreamPtr os_;
  Status status_;
  int indent_ = 0;
  void Init() {
    os_.swap(env_->output_system()->fopen(output_));
  }
};

}  // namespace juicyc

#endif  // JUICYC_JSON_DUMPER_H_