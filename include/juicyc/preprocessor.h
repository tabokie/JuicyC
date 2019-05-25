#ifndef JUICYC_PREPROCESSOR_H_
#define JUICYC_PREPROCESSOR_H_

#include "compiler.h"
#include "env.h"
#include "util/util.h"

#include <memory>

namespace juicyc {

class Preprocessor : public NoMove {
 public:
 	virtual ~Preprocessor() {}
 	// io
 	virtual bool eof() const = 0;
 	virtual Status status() const = 0;
 	virtual bool good() const = 0;
 	virtual bool ok() const = 0;
 	virtual char get() = 0;
 	// cursor
 	virtual std::string file_name() const = 0;
 	virtual uint32_t line_no() const = 0;
 	virtual uint32_t col_no() const = 0;
 	// task scheduler
 	virtual bool seek() = 0;
 	virtual bool next() = 0;
 	virtual void push(std::string& file) = 0;
 	static std::unique_ptr<Preprocessor> NewPreprocessor(CompilerOption&, Env*);
};

} // namespace juicyc
#endif // JUICYC_PREPROCESSOR_H_