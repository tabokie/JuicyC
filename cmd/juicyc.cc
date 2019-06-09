#include "juicyc/compiler.h"
#include "juicyc/status.h"

using namespace juicyc;

int main(int argc, char** argv) {

  // parse option
  CompilerOption opt;

  auto p = Compiler::NewCompiler(opt);
  if(!p) {
    return 1;
  }

  auto s = p->Run();
  if(!s.ok()) {
    return 1;
  }

  return 0;
}