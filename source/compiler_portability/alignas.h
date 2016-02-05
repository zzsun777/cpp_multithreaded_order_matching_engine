#ifndef _ALIGN_AS_
#define _ALIGN_AS_

// Use C++11 if available , otherwise compiler extensions
#if defined(_MSC_VER)
#if MSC_VER<=1800
#define alignas(A) __declspec(align(A))
#endif
#elif defined(__GNUC__)
#if (__GNUC__==4) and (__GNUC_MINOR__ <= 7) 
#define alignas(A) __attribute__((aligned(A)))
#endif
#endif

#endif