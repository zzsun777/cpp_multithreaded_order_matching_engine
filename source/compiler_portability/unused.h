#ifndef _UNUSED_H_
#define _UNUSED_H_

//To avoid unused variable warnings
#if defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(_MSC_VER)
#define UNUSED(x) __pragma(warning(suppress:4100)) x
#endif

// To encourage devs avoiding checking error results
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RESULT _Check_return_
#else
#define CHECK_RESULT
#endif

#endif