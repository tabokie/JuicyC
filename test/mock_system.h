#ifndef JUICYC_TEST_MOCK_SYSTEM_H_
#define JUICYC_TEST_MOCK_SYSTEM_H_

#include "juicyc/system.h"

#include <unordered_map>
#include <istream>
#include <sstream>
#include <memory>

namespace juicyc {

namespace test {

class MockInputSystem : public InputSystem {
 protected:
  std::unordered_map<std::string, std::string> filesys_;
 public:
  void set_file(const std::string& file, const std::string& content) {
    filesys_.insert(std::make_pair(file, content));
  }
  virtual IStreamPtr fopen(const std::string& file) {
    auto p = filesys_.find(file);
    if(p == filesys_.end()) {
      return MakeStream(std::move(std::istringstream("")));
    }
    return MakeStream(std::move(std::istringstream(p->second)));
  }
};

class MockOutputSystem : public OutputSystem {
 protected:
  std::unordered_map<std::string, std::string> filesys_;
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
    p.release();
    size_t idx = str.find(':');
    if(idx == std::string::npos) return ; // invalid
    filesys_.insert(std::make_pair(str.substr(0, idx), str.substr(idx + 1)));
  }
};

} // namespace test

} // namespace juicyc

#endif // JUICYC_TEST_MOCK_SYSTEM_H_