#ifndef __CACHE_LINE__
#define __CACHE_LINE__

#define CACHE_LINE_SIZE 64
#define CACHE_ALIGNED alignas(CACHE_LINE_SIZE)

#endif