#ifndef _ALIGNED_MEMORY_H_
#define _ALIGNED_MEMORY_H_

#include "cache_line.h"
#include <compiler_portability/noexcept.h>
#include <cstddef>
#include <new>

namespace memory
{

void* alignedMalloc(std::size_t sz, std::size_t alignment = CACHE_LINE_SIZE) throw(std::bad_alloc);
void alignedFree(void* ptr) noexcept;

} // namespace

#endif