#include "front_env.h"

juicyc::UniqueSymtable juicyc::FrontEnv::tagger_;
std::unique_ptr<juicyc::Preprocessor> juicyc::FrontEnv::pp = nullptr;