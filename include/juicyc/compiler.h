#ifndef JUICYC_COMPILER_H_
#define JUICYC_COMPILER_H_

#include "status.h"
#include "system.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace juicyc {

struct CompilerOptions {
  std::vector<std::string> files;
  std::string dump_output;
  std::string output;
  // optional
  InputSystem* isys = nullptr;
  OutputSystem* osys = nullptr;
  CompilerOptions() = default;
  CompilerOptions(const CompilerOptions& rhs)
      : files(rhs.files),
        dump_output(rhs.dump_output),
        output(rhs.output),
        isys(rhs.isys),
        osys(rhs.osys) {}
  ~CompilerOptions() { }
};

class Compiler {
 public:
  Compiler() = default;
  ~Compiler() { }
  // preprocessor + scanner + parser
  virtual Status Parse() = 0;
  virtual Status GenerateIR() = 0;
  virtual Status GenerateAsm() = 0;
  static std::unique_ptr<Compiler> NewCompiler(CompilerOptions option);
};

} // namespace juicyc

#endif // JUICYC_COMPILER_H_