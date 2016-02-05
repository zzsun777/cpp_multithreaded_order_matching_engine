#ifndef __ALIGNED_MEMORY_H__
#define __ALIGNED_MEMORY_H__

#include "cache_line.h"
#include <cstddef>

namespace memory
{

void* alignedMalloc(std::size_t sz, std::size_t alignment = CACHE_LINE_SIZE);
void alignedFree(void* ptr);

} // namespace

#endif