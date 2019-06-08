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
    if (!block_head_) {
      (*os_) << ",\n";
    } else block_head_ = false;
    (*os_) << std::string(indent_, ' ')
           << "\"value\": "
           // << static_cast<int>(n->type)
           << "\"" << (n->value) << "\"";
  }
  void ExitTerminal(Terminal* n) override {}
  void VisitNonTerminal(NonTerminal* n) override {
    if (!block_head_) {
      (*os_) << ",\n";
    } else block_head_ = false;
    (*os_) << std::string(indent_, ' ')
           << "\""
           << FrontEnv::Untag(n->type) << "\": {\n";
    indent_ += 2;
    block_head_ = true;
  }
  void ExitNonTerminal(NonTerminal* n) override {
    indent_ -= 2;
    block_head_ = false;
    (*os_) << "\n" << std::string(indent_, ' ') <<  "}";
  }
  Status status() const override {
    return status_;
  }
 protected:
  std::string output_;
  Env* env_;
  OStreamPtr os_;
  Status status_;

  bool block_head_ = true;
  int indent_ = 0;
  void Init() {
    os_ = env_->output_system()->fopen(output_);
  }
};

}  // namespace juicyc

#endif  // JUICYC_JSON_DUMPER_H_