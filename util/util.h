#ifndef JUICYC_UTIL_UTIL_H_
#define JUICYC_UTIL_UTIL_H_

namespace juicyc {

class NoCopy {
 public:
  NoCopy() = default;
  NoCopy(const NoCopy&) = delete;
  NoCopy& operator=(const NoCopy&) = delete;
};

class NoMove {
 public:
  NoMove() = default;
  NoMove(const NoMove&) = delete;
  NoMove& operator=(const NoMove&) = delete;
  NoMove(NoMove&&) = delete;
  NoMove& operator=(NoMove&&) = delete;
};

} // namespace juicyc

#endif // JUICYC_UTIL_UTIL_H_