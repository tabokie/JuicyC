#ifndef JUICYC_COMPILER_H_
#define JUICYC_COMPILER_H_

#include "status.h"

#include <string>
#include <vector>
#include <memory>

namespace juicyc {

struct CompilerOption {
  std::vector<std::string> files;
  std::string output;
  CompilerOption() = default;
  CompilerOption(const CompilerOption& rhs) : 
      files(rhs.files),
      output(rhs.output) { }
  ~CompilerOption() { }
};

class Compiler {
 public:
  Compiler() = default;
  ~Compiler() { }
  // preprocessor + scanner + parser
  virtual Status Parse() = 0;
  virtual Status GenerateIR() = 0;
  virtual Status GenerateAsm() = 0;
  static std::unique_ptr<Compiler>
  NewCompiler(CompilerOption& option);
};

} // namespace juicyc

#endif // JUICYC_COMPILER_H_