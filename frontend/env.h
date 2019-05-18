#ifndef JUICYC_FRONTEND_ENV_H_
#define JUICYC_FRONTEND_ENV_H_

#include "util/util.h"
#include "util/unique_symtable.h"
#include "juicyc/preprocessor.h"

namespace juicyc {

// Env is single instance collection
// that is designed to support lex/yacc
// runtime operation.
class Env : public NoMove {
 protected:
 	static UniqueSymtable tagger_;
 public:
 	static std::unique_ptr<Preprocessor> pp;
 	static uint16_t Tag(std::string& key) {
 		return tagger_.Insert(key);
 	}
};

} // namespace juicyc

#endif // JUICYC_FRONTEND_ENV_H_