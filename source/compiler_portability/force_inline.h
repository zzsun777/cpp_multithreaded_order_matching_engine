#ifndef __FORCE_INLINE__
#define __FORCE_INLINE__

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__)
#define FORCE_INLINE __attribute__((always_inline))
#endif

#endif