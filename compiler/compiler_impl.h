#ifndef JUICYC_CMD_COMPILER_IMPL_H_
#define JUICYC_CMD_COMPILER_IMPL_H_

#include "juicyc/compiler.h"
#include "juicyc/preprocessor.h"
#include "frontend/front_env.h"
#include "visitor/json_dumper.h"
#include "visitor/syntax_visitor.h"

namespace juicyc {

class CompilerImpl : public Compiler {
 public:
  CompilerImpl(CompilerOptions opts) : opts_(opts) { Init(); }
  ~CompilerImpl() { }
  // preprocessor + scanner + parser
  Status Run() override {
    FrontEnv::pp->seek();
  	auto s = FrontEnv::Parse();
  	if (s.ok() &&
  		  FrontEnv::root) {
      if (s.ok() && opts_.json_output.size() > 0) {
        JsonDumper dumper(opts_.json_output, &env_);
        FrontEnv::root->Invoke(dumper);
        s = dumper.status();
      }
      if (opts_.ir_output.size() > 0 || opts_.obj_output.size() > 0) {
        SyntaxVisitor visitor(&env_);
        FrontEnv::root->Invoke(visitor);
        s = visitor.status();
        if (s.ok() && opts_.ir_output.size() > 0) {
          s = visitor.context().ExportIR(opts_.ir_output);
        }
        if (s.ok() && opts_.obj_output.size() > 0) {
          s = visitor.context().ExportObj(opts_.obj_output);
        }  
      }
  	}
  	return s;
  }
 private:
 	CompilerOptions opts_;
 	Env env_;
 	void Init() {
 		if (!opts_.isys) opts_.isys = new InputSystem();
 		if (!opts_.osys) opts_.osys = new OutputSystem();
 		env_.set_input_system(opts_.isys);
 		env_.set_output_system(opts_.osys);
 		FrontEnv::pp = Preprocessor::NewPreprocessor(opts_, &env_);
 	}
};

} // namespace juicyc

#endif // JUICYC_CMD_COMPILER_IMPL_H_