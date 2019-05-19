#ifndef JUICYC_UTIL_UNIQUE_SYMTABLE_H_
#define JUICYC_UTIL_UNIQUE_SYMTABLE_H_

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace juicyc {

class UniqueSymtable {
  // Not thread-safe for now.
  // Currently a lousy implementation that stores
  // two copies of key.
  std::vector<std::string> vec_;
  std::unordered_map<std::string, uint16_t> map_;
 public:
  uint16_t Insert(std::string& key) {
    uint16_t ret = Get(key);
    if(ret > 0) return ret;
    vec_.push_back(key);
    ret = vec_.size();
    map_.insert(std::make_pair(key, ret));
    return ret;
  }
  // accept positive id
  std::string& Get(uint16_t id) {
    return vec_[id-1];
  }
  const std::string& Get(uint16_t id) const {
    return vec_[id-1];
  }
  uint16_t Get(std::string& key) {
    auto p = map_.find(key);
    if(p == map_.end()) return 0;
    return p->second;
  }
};

} // namespace juicyc

#endif // JUICYC_UTIL_UNIQUE_SYMTABLE_H_