#ifndef _CACHE_LINE_H_
#define _CACHE_LINE_H_

#include <compiler_portability/alignas.h>

#define CACHE_LINE_SIZE 64
#define CACHE_ALIGNED alignas(CACHE_LINE_SIZE)

#endif