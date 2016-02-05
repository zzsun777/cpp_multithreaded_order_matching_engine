#ifndef __NO_INLINE__
#define __NO_INLINE__

#if defined(_MSC_VER)
#define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NO_INLINE __attribute__((noinline))
#endif

#endif