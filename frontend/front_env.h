#ifndef JUICYC_FRONTEND_FRONT_ENV_H_
#define JUICYC_FRONTEND_FRONT_ENV_H_

#include "juicyc/symbol.h"
#include "gen_parse.hh"
#include "util/util.h"
#include "util/unique_symtable.h"
#include "juicyc/status.h"
#include "juicyc/preprocessor.h"

namespace juicyc {

// FrontEnv is single instance collection
// that is designed to support lex/yacc
// runtime operation.
class FrontEnv : public NoMove {
 protected:
  static UniqueSymtable tagger_;
 public:
  static std::unique_ptr<Preprocessor> pp;
  static Status status;
  static Symbol* root;
  static uint16_t Tag(std::string& key) {
    return tagger_.Insert(key);
  }
  static uint16_t Tag(const char* key) {
    return tagger_.Insert(std::string(key));
  }
  static std::string Untag(uint16_t id) {
    return tagger_.Get(id);
  }
  static Status Parse() {
    yyparse();
    return status;
  }
};

} // namespace juicyc

#endif // JUICYC_FRONTEND_FRONT_ENV_H_