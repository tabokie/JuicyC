#ifndef JUICYC_UTIL_SPIN_LOCK_H_
#define JUICYC_UTIL_SPIN_LOCK_H_

#include "util.h"

#include <atomic>

namespace juicyc {

class SpinLock : public NoMove {
 public:
  SpinLock() { }
  ~SpinLock() { }
  void Lock() {
    bool tmp = false;
    while(true) {
      if(!flag_.load() && 
          std::atomic_compare_exchange_strong(
            &flag_,
            &tmp,
            true
            )
          )
        break;
      tmp = false;
    }
  }
  void Unlock() {
    flag_.store(false);
  }
 private:
  std::atomic<bool> flag_ = false;
};

class SpinLockHandle : public NoCopy {
 public:
  SpinLockHandle() { } // dummy lock
  SpinLockHandle(std::atomic<bool>& lock): ref_(&lock) {
    bool tmp = false;
    while(true) {
      if(!flag_.load() && 
          std::atomic_compare_exchange_strong(
            &flag_,
            &tmp,
            true
            )
          ) {
        break;
      }
      tmp = false;
    }
  }
  SpinLockHandle(SpinLockHandle&& rhs): ref_(rhs.ref_) {
    rhs.ref_ = NULL;
  }
  ~SpinLockHandle() {
    if(ref_) {
      assert(ref_->load());
      ref_->store(false); 
    }
  }
 private:
  std::atomic<bool>* ref_ = NULL;
};

} // namespace juicyc

#endif // JUICYC_UTIL_SPIN_LOCK_H_