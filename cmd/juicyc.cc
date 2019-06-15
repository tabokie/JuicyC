#include "juicyc/compiler.h"
#include "juicyc/status.h"
#include "juicyc/system.h"

#include <memory>
#include <string>
#include <iostream>

using namespace juicyc;

int main(int argc, char** argv) {

  // parse option
  CompilerOptions opt;
  bool output_json = false;
  bool output_llvm = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'j') output_json = true;
      else if (argv[i][1] == 'l') output_llvm = true;
    } else {
      opt.files.push_back(std::string(argv[i]));
    }
  }
  if (opt.files.size() == 0) {
    std::cout << "Specify at least one input file";
    exit(1);
  }
  std::unique_ptr<InputSystem> pi(new InputSystem());
  std::unique_ptr<OutputSystem> po(new OutputSystem());
  opt.isys = pi.get();
  opt.osys = po.get();
  int idx = opt.files.front().find('.');
  std::string prefix;
  if (idx != std::string::npos) {
    prefix = opt.files.front().substr(0, idx);
  } else {
    prefix = opt.files.front();
  }
  opt.obj_output = prefix + ".obj";
  if (output_json) {
    opt.json_output = prefix + ".json";
  }
  if (output_llvm) {
    opt.ir_output = prefix + ".ll";
  }

  auto p = Compiler::NewCompiler(opt);
  if(!p) {
    std::cout << "Fatal: failed to initialize new compiler" << std::endl;
    exit(1);
  }

  auto s = p->Run();
  if(!s.ok()) {
    std::cout << s.ToString() << std::endl;
    exit(1);
  }
  return 0;
}