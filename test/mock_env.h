#ifndef JUICYC_TEST_MOCK_ENV_H_
#define JUICYC_TEST_MOCK_ENV_H_

#include "juicyc/env.h"
#include "juicyc/system.h"

#include <unordered_map>
#include <istream>
#include <sstream>
#include <memory>
#include <iostream>

namespace juicyc {

namespace test {

class MockInputSystem : public InputSystem {

 public:
  void set_file(const std::string& file, const std::string& content) {
    filesys_[file] = content;
  }
  virtual IStreamPtr fopen(const std::string& file) {
    auto p = filesys_.find(file);
    if(p == filesys_.end()) {
      return MakeStream(std::move(std::istringstream("")));
    }
    return MakeStream(std::move(std::istringstream(p->second)));
  }

 protected:
  std::unordered_map<std::string, std::string> filesys_;
};

class MockOutputSystem : public OutputSystem {

 public:
  std::string get_file(const std::string& file) {
    auto p = filesys_.find(file);
    if(p == filesys_.end()) return "";
    return p->second;
  }
  virtual OStreamPtr fopen(const std::string& file) {
    auto s = std::ostringstream();
    s << file << ":" ;
    return MakeStream(std::move(s));
  }
  virtual void fclose(OStreamPtr& p) {
    std::string str = reinterpret_cast<std::ostringstream*>(p.get())->str();
    size_t idx = str.find(':');
    if(idx == std::string::npos) return ; // invalid
    filesys_[str.substr(0, idx)] = str.substr(idx + 1);
  }

 protected:
  std::unordered_map<std::string, std::string> filesys_;
};

class MockIStreamPtr : public IStreamPtr {

 public:
  MockIStreamPtr(IStreamPtr&& rhs, MockInputSystem* sys)
      : IStreamPtr(std::move(rhs)),
        sys_(sys) {}
  MockIStreamPtr(std::istream* p, MockInputSystem* sys)
      : IStreamPtr(p),
        sys_(sys) {}
  ~MockIStreamPtr() {}

 protected:
  MockInputSystem* sys_ = nullptr;
};

class MockEnv : public Env {

 public:
  MockEnv() {
    input_system_ = (std::make_unique<MockInputSystem>());
    output_system_ = (std::make_unique<MockOutputSystem>());
  }
};

} // namespace test

} // namespace juicyc

#endif // JUICYC_TEST_MOCK_ENV_H_