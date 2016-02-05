#ifndef _HOOK_MS_CRT_H_
#define _HOOK_MS_CRT_H_

#include <cstddef>

namespace memory
{

namespace debugging
{
#if _WIN32 && _DEBUG
// https://msdn.microsoft.com/en-us/library/cy8c7wz5.aspx
#include <crtdbg.h>

// In order to use you have to uncomment _CRTDBG_MAP_ALLOC line 
// and also call hook_crt

//#define _CRTDBG_MAP_ALLOC 

inline void hook_crt()
{
    _CrtSetAllocHook(crt_malloc_hook);
}

inline static int crt_malloc_hook(int allocType, void *data, std::size_t size, int blockUse, long request, const unsigned char * fileName, int line)
{
    if (blockUse == _CRT_BLOCK)
    {
        return 1;
    }

    switch (allocType)
    {
        case _HOOK_ALLOC:
            break;

        case _HOOK_REALLOC:
            break;

        case _HOOK_FREE:
            break;
    }

    return 1;
}

#endif
}//debugging
}//namespace

#endif