#ifndef __FASTCALL__
#define __FASTCALL__

#if defined(_MSC_VER)
#if _MSC_VER >= 1800
#define fastcall __fastcall
#else
#define fastcall __vectorcall
#endif
#elif defined(__GNUC__)
#define fastcall __attribute__((fastcall))
#endif

#endif