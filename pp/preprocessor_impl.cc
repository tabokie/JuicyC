#include "preprocessor_impl.h"

#include <memory>

namespace juicyc {

std::unique_ptr<Preprocessor> Preprocessor::NewPreprocessor(CompilerOption& opts, Env* env) {
  return std::make_unique<PreprocessorImpl>(opts, env);
}

}  // namespace juicyc