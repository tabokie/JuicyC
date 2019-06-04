#include "compiler_impl.h"

namespace juicyc {

std::unique_ptr<Compiler> Compiler::NewCompiler(CompilerOptions opts) {
  return std::make_unique<CompilerImpl>(opts);
}

} // namespace juicyc