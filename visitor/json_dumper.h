#ifndef JUICYC_JSON_DUMPER_H_
#define JUICYC_JSON_DUMPER_H_

#include "juicyc/env.h"
#include "juicyc/symbol_visitor.h"
#include "juicyc/symbol.h"

#include <string>

namespace juicyc {

std::string filter(std::string& str) {
  std::string ret;
  if (str[0] == '\"') {
    ret = std::string("\\\"") + str.substr(1,str.size()-2) + std::string("\\\"");
  } else {
    return str;
  }
  return ret;
}

class JsonDumper : public SymbolVisitor {
 public:
  JsonDumper(std::string output, Env* env)
      : output_(output),
        env_(env) { Init(); }
  ~JsonDumper() {
    (*os_) << "\n}";
    env_->output_system()->fclose(os_);
  }
  void VisitTerminal(Terminal* n) override {
    if (!block_head_) {
      (*os_) << ",\n";
    } else block_head_ = false;
    (*os_) << std::string(indent_, ' ')
           << "\"terminal-" << (terminal_id_++) << "\": {\n";
    (*os_) << std::string(indent_ + 2, ' ')
           << "\"value\": \"" << filter(n->value) << "\",\n";
    (*os_) << std::string(indent_ + 2, ' ')
           << "\"file\": \"" << (n->file) << "\",\n";
    (*os_) << std::string(indent_ + 2, ' ')
           << "\"line\": \"" << (n->line) << "\"\n";
    (*os_) << std::string(indent_, ' ') << "}";
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
  uint64_t terminal_id_ = 0;

  bool block_head_ = true;
  int indent_ = 2;
  void Init() {
    os_ = env_->output_system()->fopen(output_);
    (*os_) << "{\n";
  }
};

}  // namespace juicyc

#endif  // JUICYC_JSON_DUMPER_H_