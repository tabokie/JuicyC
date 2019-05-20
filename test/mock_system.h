#ifndef JUICYC_TEST_MOCK_SYSTEM_H_
#define JUICYC_TEST_MOCK_SYSTEM_H_

#include "juicyc/system.h"

#include <unordered_map>
#include <istream>
#include <sstream>

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

} // namespace test

} // namespace juicyc

#endif // JUICYC_TEST_MOCK_SYSTEM_H_