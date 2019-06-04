#include "preprocessor_impl.h"

#include <memory>

namespace juicyc {

std::unique_ptr<Preprocessor> Preprocessor::NewPreprocessor(
	    CompilerOptions opts, Env* env) {
  return std::make_unique<PreprocessorImpl>(opts, env);
}

}  // namespace juicyc