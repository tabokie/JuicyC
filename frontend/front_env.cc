#include "front_env.h"

namespace juicyc {

UniqueSymtable FrontEnv::tagger_;
std::unique_ptr<Preprocessor> FrontEnv::pp = nullptr;
Symbol* FrontEnv::root = nullptr;
Status FrontEnv::status;

}  // namespace juicyc
