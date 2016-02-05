#ifndef _PREPROCESSOR_
#define _PREPROCESSOR_

#if defined(_MSC_VER)
#ifndef __func__
#if _MSC_VER < 1900 // pre-MSVC2015 are not fully compliant with C99
#define __func__ __FUNCTION__
#endif
#endif
#endif

#endif