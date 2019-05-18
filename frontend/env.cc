#include "env.h"

juicyc::UniqueSymtable juicyc::Env::tagger_;
std::unique_ptr<juicyc::Preprocessor> juicyc::Env::pp = nullptr;