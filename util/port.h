#ifndef JUICYC_UTIL_PORT_H_
#define JUICYC_UTIL_PORT_H_

// system platform
#if defined(__WIN32) || defined(__WIN64) || defined(_MSC_VER)
	#define WIN_PLATFORM
#elif defined(__linux)
	#define LINUX_PLATFORM
#else
	#define UNSPEC_PLATFORM
#endif

#ifdef WIN_PLATFORM
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Windows.h"
#elif defined(LINUX_PLATFORM)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "ucontext.h"
#endif

// assume little endian
#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN
#endif

#endif // JUICYC_UTIL_PORT_H_