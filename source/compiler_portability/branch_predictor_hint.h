#ifndef _BRANCH_PREDICTOR_HINT_
#define _BRANCH_PREDICTOR_HINT_

#if defined(_MSC_VER)
//No implementation provided for MSVC :
//https://social.msdn.microsoft.com/Forums/vstudio/en-US/2dbdca4d-c0c0-40a3-993b-dc78817be26e/branch-hints?forum=vclanguage
#define likely(x)
#define unlikely(x)
#elif defined(__GNUC__)
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#endif