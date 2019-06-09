#ifndef JUICYC_UTIL_UTIL_H_
#define JUICYC_UTIL_UTIL_H_

#include <cstring>
#include <vector>
#include <iostream>
#include <string>

namespace juicyc {

class NoCopy {
 public:
  NoCopy() = default;
  NoCopy(const NoCopy&) = delete;
  NoCopy& operator=(const NoCopy&) = delete;
  virtual ~NoCopy() {}
};

class NoMove {
 public:
  NoMove() = default;
  NoMove(const NoMove&) = delete;
  NoMove& operator=(const NoMove&) = delete;
  NoMove(NoMove&&) = delete;
  NoMove& operator=(NoMove&&) = delete;
  virtual ~NoMove() {}
};

// compiler branch prediction //
#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       x
#define unlikely(x)     x
#endif

struct FunctionGuard {
 public:
  FunctionGuard(std::ostream& os, const char* name)
      : os_(os),
        name_(name) {
    os_ << "entering " << name_ << std::endl;
  }
  FunctionGuard(std::ostream& os, const std::string name)
      : os_(os),
        name_(name) {
    os_ << "entering " << name_ << std::endl;
  }
  ~FunctionGuard() {
    os_ << "exiting " << name_ << std::endl;
  }
 private:
  std::ostream& os_;
  std::string name_;
};

struct StringUtil {

  static int split(std::string& str,
                   std::vector<std::string>& ret,
                   int max_partition = -1) {
    int cur = 0;
    int to = 0;
    int count = ret.size();
    while((max_partition <= 0 ||
           ret.size() - count < max_partition - 1) &&
          (cur = str.find_first_not_of(' ', cur)) != std::string::npos) {
      to = str.find_first_of(' ', cur); // find space
      if(to == std::string::npos) to = str.size();
      if(to > cur) {
        ret.push_back(str.substr(cur, to - cur));
      }
      cur = to;
    }
    if(cur < str.size()) ret.push_back(str.substr(cur)); // don;t split tail
    return ret.size() - count;
  }

  static int split(std::string& str,
                   std::string token,
                   std::vector<std::string>& ret) {
    int cur = 0;
    int to = 0;
    int count = ret.size();
    while((cur = str.find_first_not_of(token, cur)) != std::string::npos) {
      to = str.find_first_of(token, cur); // find space
      if(to == std::string::npos) to = str.size();
      if(to > cur) {
        ret.push_back(str.substr(cur, to - cur));
      }
      cur = to + 1;
    }
    return ret.size() - count;
  }

  static bool starts_with(std::string& a, std::string b) {
    if(a.size() < b.size()) return false;
    return a.compare(0, b.size(), b) == 0;
  }

  static bool ends_with(std::string& a, std::string b) {
    if(a.size() < b.size()) return false;
    return b.compare(0, b.size(),
                     a.c_str() + a.size() - b.size(),
                     b.size()) == 0;
  }

  static bool starts_with(const char* p, std::string b, int len) {
    register int i = 0;
    while (p[i] && i < b.size() && i < len) {
      if (p[i] != b[i]) return false;
      ++i;
    }
    return i >= b.size();
  }

  static bool ends_with(const char* p, std::string b, int len) {
    return starts_with(p + len - b.size(), b, len - b.size());
  }

  static std::string identifier(const char* p, int len) {
    register int end = 0;
    register char c;
    bool met_quote0 = false;
    bool met_quote1 = false;
    while ((c = p[end]) && end < len) {
      if (!(met_quote0 || met_quote1 ||
          c >= '0' && c <= '9' ||
          c >= 'A' && c <= 'Z' ||
          c >= 'a' && c <= 'z' ||
          c == '_' || c == '\'' || c == '\"')) break; 
      if (c == '\'') {
        if (met_quote0) {
          ++end;
          break;
        } else {
          met_quote0 = true;
        }
      } else if (c == '\"') {
        if (met_quote1) {
          ++ end;
          break;
        } else {
          met_quote1 = true;
        }
      }
      ++end;
    }
    return std::string(p, end);
  }
};



class Logger {

 public:
  void trace(std::string context) {
    trace_buffer_[head_++] = context;
    if (head_ >= trace_buffer_size_) head_ = 0;
  }
  void output_trace(std::ostream& os) {
    for (int i = 0; i < trace_buffer_size_; i++) {
      os << "[" << std::to_string(i) << "]"
         << trace_buffer_[(head_ + i) % trace_buffer_size_]
         << std::endl;
    }
  }
  void set_cursor(std::string c) { cursor_ = c; }
  void warning(std::string message) {
    warnings_.push_back(cursor_ + message);
  }
  int num_warning() { return warnings_.size(); }
  void output_warning(std::ostream& os) {
    for (auto& w : warnings_) {
      os << "warning: " << w << std::endl;
    }
  }
  void error(std::string message) {
    std::cout << message << std::endl;
    errors_.push_back(cursor_ + message);
  }
  int num_error() { return errors_.size(); }
  void output_error(std::ostream& os) {
    for (auto& e : errors_) {
      os << "error: " << e << std::endl;
    }
  }
  void clear() {
    head_ = 0;
    for (int i = 0; i < trace_buffer_size_; i++)
      trace_buffer_[i] = "";
    warnings_.clear();
    errors_.clear();
  }
 private:
  static constexpr int16_t trace_buffer_size_ = 16;
  std::string trace_buffer_[trace_buffer_size_];
  int head_ = 0;
  std::string cursor_;
  std::vector<std::string> warnings_;
  std::vector<std::string> errors_;
};

} // namespace juicyc

#endif // JUICYC_UTIL_UTIL_H_