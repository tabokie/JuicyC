#ifndef JUICYC_CMD_COMPILER_IMPL_H_
#define JUICYC_CMD_COMPILER_IMPL_H_

#include "juicyc/compiler.h"
#include "juicyc/preprocessor.h"
#include "frontend/front_env.h"
#include "visitor/json_dumper.h"

namespace juicyc {

class CompilerImpl : public Compiler {
 public:
  CompilerImpl(CompilerOptions opts) : opts_(opts) { Init(); }
  ~CompilerImpl() { }
  // preprocessor + scanner + parser
  Status Parse() override {
    FrontEnv::pp->seek();
  	auto s = FrontEnv::Parse();
  	if (s.ok() &&
  		  FrontEnv::root &&
  		  opts_.dump_output.size() > 0) {
  		JsonDumper dumper(opts_.dump_output, &env_);
  		FrontEnv::root->Invoke(dumper);
  		return dumper.status();
  	}
  	return s;
  }
  Status GenerateIR() override {
  	return Status::NotSupported("not implemented");
  }
  Status GenerateAsm() override {
  	return Status::NotSupported("not implemented");
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