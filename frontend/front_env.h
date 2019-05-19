#ifndef JUICYC_FRONTEND_FRONT_ENV_H_
#define JUICYC_FRONTEND_FRONT_ENV_H_

#include "util/util.h"
#include "util/unique_symtable.h"
#include "juicyc/status.h"
#include "juicyc/preprocessor.h"
#include "parse.hh"

namespace juicyc {

// FrontEnv is single instance collection
// that is designed to support lex/yacc
// runtime operation.
class FrontEnv : public NoMove {
 protected:
 	static UniqueSymtable tagger_;
 public:
 	static std::unique_ptr<Preprocessor> pp;
 	static uint16_t Tag(std::string& key) {
 		return tagger_.Insert(key);
 	}
 	static Status Parse() {
 		yyparse();
 	}
};

} // namespace juicyc

#endif // JUICYC_FRONTEND_FRONT_ENV_H_