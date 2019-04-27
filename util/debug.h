#ifndef JUICYC_UTIL_DEBUG_H_
#define JUICYC_UTIL_DEBUG_H_

#include "port.h"
#include "spinlock.h"

namespace juicyc {

#ifdef JUICYC_DEBUG
  #define COMMENT(str)          std::cout << __LINE__ << ": " << str << std::endl;
#else
  #define COMMENT(str)
#endif

namespace {

// colored logging
class ThreadInfo{
 public:
  static std::string thread_string(void){
    return std::string("[thread ") + std::to_string(thread_id()) + "]";
  }
  static int thread_id(void){
    static std::atomic<int> id(0);
    thread_local int thread_id = std::atomic_fetch_add(&id, 1);
    return thread_id;
  }
};

#ifdef WIN_PLATFORM

struct ConsoleColor {
  enum ConsoleForegroundColor {
    enmCFC_Red          = FOREGROUND_INTENSITY | FOREGROUND_RED,
    enmCFC_Green        = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    enmCFC_Blue         = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    enmCFC_Yellow       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    enmCFC_Purple       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    enmCFC_Cyan         = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    enmCFC_Gray         = FOREGROUND_INTENSITY,
    enmCFC_White        = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    enmCFC_HighWhite    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    enmCFC_Black        = 0,
  };
  enum ConsoleBackGroundColor {
    enmCBC_Red          = BACKGROUND_INTENSITY | BACKGROUND_RED,
    enmCBC_Green        = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
    enmCBC_Blue         = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
    enmCBC_Yellow       = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
    enmCBC_Purple       = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
    enmCBC_Cyan         = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
    enmCBC_White        = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
    enmCBC_HighWhite    = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
    enmCBC_Black        = 0,
  };
  void operator()(int code){
    static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    switch(code){
      case 0:SetConsoleTextAttribute(handle, enmCFC_White | enmCBC_Black);break;
      case 1:SetConsoleTextAttribute(handle, enmCFC_HighWhite | enmCBC_Black);break;
      case 2:SetConsoleTextAttribute(handle, enmCFC_Black | enmCBC_Yellow);break;
      case 3:SetConsoleTextAttribute(handle, enmCFC_Black | enmCBC_Red);break;
      // red
      case 4:SetConsoleTextAttribute(handle, enmCFC_Red | enmCBC_Black);break;
      // green
      case 5:SetConsoleTextAttribute(handle, enmCFC_Green | enmCBC_Black);break;
      // blue
      case 6:SetConsoleTextAttribute(handle, enmCFC_Blue | enmCBC_Black);break;
      // cyan
      case 7:SetConsoleTextAttribute(handle, enmCFC_Cyan | enmCBC_Black);break;
      // purple
      case 8:SetConsoleTextAttribute(handle, enmCFC_Purple | enmCBC_Black);break;
      // yellow
      case 9:SetConsoleTextAttribute(handle, enmCFC_Yellow | enmCBC_Black);break;
    }
  }
};

#endif // WIN_PLATFORM

class Colog: public NoMove{
  SpinLock lk;
 public:
  Colog(){
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
  }
  template <typename T>
  Colog& operator<<(const T& x){
    SpinLockHandle handle(&lk);
    std::cout << ThreadInfo::thread_string() << x << std::endl;
    return *this;
  }
  template <class ...Args>
  Colog& put_batch(Args... xs){
    SpinLockHandle handle(&lk);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    return *this;
  }
#ifdef WIN_PORT
  template <class ...Args>
  Colog& highlight(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(1);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& warning(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(2);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& error(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(3);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& red(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(4);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& green(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(5);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& blue(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(6);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& cyan(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(7);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& purple(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(8);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
  template <class ...Args>
  Colog& yellow(Args... xs){
    SpinLockHandle handle(&lk);
    ConsoleColor{}(9);
    std::cout << ThreadInfo::thread_string();
    int temp[] = { ((std::cout << xs), std::cout <<" " , 0 )... };
    std::cout << std::endl;
    ConsoleColor{}(0);
    return *this;
  }
 #else  
 #warning "colored log is not implemented"
 #endif
};

}

} // namespace juicyc

#endif // JUICYC_UTIL_DEBUG_H_