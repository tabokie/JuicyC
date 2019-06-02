#ifndef JUICYC_PP_PREPROCESSOR_IMPL_H_
#define JUICYC_PP_PREPROCESSOR_IMPL_H_

#include "juicyc/preprocessor.h"
#include "juicyc/compiler.h"

#include <unordered_set>
#include <vector>
#include <string>

namespace juicyc {

class PreprocessorImpl: public Preprocessor {

 public:
  PreprocessorImpl(CompilerOptions& opt, Env* env)
      : input_source_(opt.files),
        sys_(env->input_system()),
        buffer_(buffer_init_size_, '\0') {}
  ~PreprocessorImpl() {}

  // eof is only set after a get returns '\0'
  bool eof() const { return eof_; }
  bool good() const { return !eof_ && status_.ok(); }
  bool ok() const { return status_.ok(); }
  Status status() const { return status_; }

  void clear() {
    eof_ = false;
    status_ = Status::OK();
    macro_table_.clear();
    source_line_ = 0;
    header_stack_.clear();
    in_omit_mode_ = false;
  }

  bool seek() {
    if (current_source_ < input_source_.size()) {
      clear();
      source_fs_.swap(sys_->fopen(input_source_[current_source_]));
    }
    return source_fs_.get() != nullptr;
  }

  // fetch next source file
  bool next() {
    if (current_source_ < input_source_.size()) {
      clear();
      current_source_ ++;
      source_fs_.swap(sys_->fopen(input_source_[current_source_]));
    }
    return current_source_ < input_source_.size();
  }

  char get() {
    if (!good()) return '\0';
    while (read_cursor_ >= write_cursor_ && good()) {
      ReadLine();
      if (good())
        ProcessLine();  // line could be consumed
    }
    if (good()) {
      return buffer_[read_cursor_++];
    } else {
      return '\0';
    }
  }

  std::string file_name() const {
    if (header_stack_.size() > 0) {
      return header_stack_.back().name;
    } else if (current_source_ < input_source_.size()){
      return input_source_[current_source_];
    } else {
      return "unknown_file";
    }
  }

  uint32_t line_no() const {
    if (header_stack_.size() > 0) {
      return header_stack_.back().line;
    } else if (current_source_ < input_source_.size()) {
      return source_line_;
    } else {
      return 0;
    }
  }

  uint32_t col_no() const {
    return read_cursor_;
  }

  void push(std::string& file) {
    input_source_.push_back(file);
  }

 protected:
  static constexpr int buffer_init_size_ = 16;

  InputSystem* sys_;

  Status status_;
  bool eof_ = false;

  IStreamPtr source_fs_ = nullptr;  // set to null when all file is processed
  std::vector<std::string> input_source_;
  uint32_t current_source_ = 0;  // source hook
  uint32_t source_line_ = 0;

  struct HeaderStack : public NoCopy {
    IStreamPtr is = nullptr;
    std::string name;
    uint32_t line = 0;
    HeaderStack() = default;
    HeaderStack(HeaderStack&& rhs)
        : is(std::move(rhs.is)),
          name(rhs.name),
          line(rhs.line) {}
    HeaderStack& operator=(HeaderStack&& rhs) {
      is = std::move(rhs.is);
      name = rhs.name;
      line = rhs.line;
      return *this;
    }
    ~HeaderStack() {}
  };
  std::vector<HeaderStack> header_stack_;

  // hold one complete line at a time
  int read_cursor_ = 0;
  int write_cursor_ = 0;  // size of valid charactor in buffer
  // std::vector<char> buffer_;
  std::vector<char> buffer_;

  // symbol table for macro processing
  std::unordered_set<std::string> macro_table_;
  bool in_omit_mode_ = false;

  // helper
  bool eof(std::istream* is) {
    return is == nullptr || is->eof() || (is->fail() && !is->bad());
  }
  bool bad(std::istream* is) {
    return is && is->bad() && !is->fail();
  }
  std::string context_desc() const {
    return std::string("file ") + file_name() +
           " line " + std::to_string(line_no()) +
           " col " + std::to_string(col_no());
  }

  void ReadLine() {
    read_cursor_ = 0;
    write_cursor_ = 0;
    std::istream* is = nullptr;
    // first, let's find a readable stream
    while (header_stack_.size() > 0) {
      is = header_stack_.back().is.get();
      if (!eof(is) || bad(is)) break;
      header_stack_.erase(header_stack_.end() - 1);
      is = nullptr;
    }
    if (bad(is)) {
      eof_ = true;  // set eof too
      status_ = Status::IOError("Preprocessor: Bad file stream");
      return;
    }
    if (is == nullptr || !is->good()) {  // try current source
      is = source_fs_.get();
    }
    if (eof(is)) {
      eof_ = true;
    } else if (bad(is)) {
      eof_ = true;  // set eof too
      status_ = Status::IOError("Preprocessor: Bad file stream");
    }
    if (!good()) {
      return;
    }

    // now read line from stream
    while (true) {
      is->getline(buffer_.data() + write_cursor_,
                  buffer_.size() - write_cursor_);
      if (is->fail() && !is->bad() && is->gcount() > 0) {
        is->clear();  // clear bits
        // have NOT encountered an '\n'
        write_cursor_ = buffer_.size() - 1;
        buffer_.resize(buffer_.size() * 2, '\0');  // resize twice as before
        continue;
      } else if (bad(is)) {
        eof_ = true;
        status_ = Status::IOError("Preprocessor: Bad file stream");
      } else {  // read one complete line
        // gcount includes \n
        write_cursor_ += is->gcount();
      }
      break;
    }
    if (good() && write_cursor_ > 1) {  // amazing bug
      // append '\n'
      if (buffer_[write_cursor_-1] == '\0') {
        buffer_[write_cursor_-1] = '\n';
      } else {  // in case the line end with an eof
        write_cursor_ ++;
        if (write_cursor_ >= buffer_.size() + 1) buffer_.push_back('\n');
        else buffer_[write_cursor_-1] = '\n';
      }
      // append '\0'
      if (write_cursor_ >= buffer_.size()) buffer_.push_back('\0');
      else buffer_[write_cursor_] = '\0';
    }
  }

  void ProcessLine() {
    if (header_stack_.size() > 0) {  // in header
      header_stack_.back().line ++;
    } else {
      source_line_ ++;
    }
    if (in_omit_mode_) {
      read_cursor_ = write_cursor_;
      return;
    }
    int read = read_cursor_;
    while (read < write_cursor_ && buffer_[read] == ' ') read ++;
    if (buffer_[read] == '\n' || read >= write_cursor_) return;
    if (StringUtil::starts_with(buffer_.data() + read,
                                  "#define ",
                                  write_cursor_ - read)) {
      read += 8;
      std::string macro = StringUtil::identifier(buffer_.data() + read,
                                                   write_cursor_ - read);
      macro_table_.insert(macro);
      // currently no value for macro
      read_cursor_ = write_cursor_;
    } else if (StringUtil::starts_with(buffer_.data() + read,
                                         "#include ",
                                         write_cursor_ - read)) {
      read += 9;
      std::string header = StringUtil::identifier(buffer_.data() + read,
                                                    write_cursor_ - read);
      if(header.size() > 0)
      if (header.size() > 2 && header[0] != '\"' ||
          header[header.size()-1] != '\"') {
        status_ = Status::Corruption(context_desc() +
                                      ": " + header +
                                      " is not a valid header file.");
        return;
      }
      header = header.substr(1, header.size() - 2);
      header_stack_.emplace_back(HeaderStack());
      header_stack_.back().is.swap(std::move(sys_->fopen(header)));
      header_stack_.back().name = header;
      read_cursor_ = write_cursor_;  // pass this line
    } else if (StringUtil::starts_with(buffer_.data() + read,
                                         "#ifdef ",
                                         write_cursor_ - read)) {
      read += 7;
      std::string macro = StringUtil::identifier(buffer_.data() + read,
                                                   write_cursor_ - read);
      if (macro_table_.count(macro) == 0)
        in_omit_mode_ = true;
      read_cursor_ = write_cursor_;
    } else if (StringUtil::starts_with(buffer_.data() + read,
                                       "#ifndef ",
                                       write_cursor_ - read)) {
      read += 8;
      std::string macro = StringUtil::identifier(buffer_.data() + read,
                                                 write_cursor_ - read);
      if (macro_table_.count(macro) > 0) 
        in_omit_mode_ = true;
      read_cursor_ = write_cursor_;
    } else if (StringUtil::starts_with(buffer_.data() + read,
                                       "#endif",
                                       write_cursor_ - read)) {
      in_omit_mode_ = false;
      read_cursor_ = write_cursor_;
    }
  }

};

}  // namespace juicyc

#endif  // JUICYC_PP_PREPROCESSOR_IMPL_H_