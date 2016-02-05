#ifndef __THREAD_LOCAL__
#define __THREAD_LOCAL__

#if defined(_MSC_VER)
#if MSC_VER<=1800
#include <xkeycheck.h> // In order to allow keyword macros
#define thread_local __declspec(thread)
#endif
#elif defined(__GNUC__)
#define thread_local __thread
#endif

#endif