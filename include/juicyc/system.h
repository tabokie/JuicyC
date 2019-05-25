#ifndef JUICYC_SYSTEM_H_
#define JUICYC_SYSTEM_H_

#include "util/util.h"

#include <istream>
#include <ostream>
#include <fstream>
#include <memory>
#include <functional>

namespace juicyc {

using IStreamPtr = std::unique_ptr<std::istream>;

using OStreamPtr = std::unique_ptr<std::ostream>;

class InputSystem : public NoCopy {
 public:
  template <typename T>
  static IStreamPtr MakeStream(T&& rhs) {
    return std::make_unique<T>(std::move(rhs));
  }
  virtual IStreamPtr fopen(const std::string& file) {
    return std::move(MakeStream(std::ifstream(file)));
  }
};

class OutputSystem : public NoCopy {
 public:
  template <typename T>
  static OStreamPtr MakeStream(T&& rhs) {
    return std::make_unique<T>(std::move(rhs));
  }
  virtual OStreamPtr fopen(const std::string& file) {
    return std::move(MakeStream(std::ofstream(file)));
  }
  virtual void fclose(OStreamPtr& p) {
    return ;
  }
};

} // namespace jjicyc

#endif // JUICYC_SYSTEM_H_