#include "aligned_memory.h"

#include "cache_line.h"

#ifdef __linux__
#include <cstdlib>
#elif _WIN32
#include <malloc.h>
#endif

namespace memory
{

void* alignedMalloc(std::size_t size, std::size_t alignment)
{
#ifdef __linux__
    void* ptr = nullptr;
    posix_memalign(&ptr, alignment, size);
    return ptr;
#elif _WIN32
    return _aligned_malloc(size, alignment);
#endif
    //For just C++11 implementation with std::align
    //See http://en.cppreference.com/w/cpp/memory/align
}

void alignedFree(void* ptr)
{
#ifdef __linux__
    free(ptr);
#elif _WIN32
    _aligned_free(ptr);
#endif
}

} // namespace