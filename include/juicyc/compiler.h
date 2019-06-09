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
  std::string json_output;
  std::string ir_output;
  std::string obj_output;
  // optional
  InputSystem* isys = nullptr;
  OutputSystem* osys = nullptr;
  CompilerOptions() = default;
  CompilerOptions(const CompilerOptions& rhs)
      : files(rhs.files),
        json_output(rhs.json_output),
        ir_output(rhs.ir_output),
        obj_output(rhs.obj_output),
        isys(rhs.isys),
        osys(rhs.osys) {}
  ~CompilerOptions() { }
};

class Compiler {
 public:
  Compiler() = default;
  ~Compiler() { }
  // preprocessor + scanner + parser
  virtual Status Run() = 0;
  static std::unique_ptr<Compiler> NewCompiler(CompilerOptions option);
};

} // namespace juicyc

#endif // JUICYC_COMPILER_H_