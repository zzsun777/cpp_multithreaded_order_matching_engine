#ifndef _HOOK_GNU_LIBC_
#define _HOOK_GNU_LIBC_

#include <cstddef>

namespace memory
{

namespace debugging
{
#if __linux__
// http://www.gnu.org/software/libc/manual/html_node/Hooks-for-Malloc.html
#include <malloc.h>

inline void init_hook (void);
static inline void *gnu_libc_malloc_hook (std::size_t, const void *);
static inline void gnu_libc__free_hook (void*, const void *);

// Uncomment the next line to set the hooks
void(*__malloc_initialize_hook) (void) = init_hook;

inline void init_hook(void)
{
    old_malloc_hook = __malloc_hook;
    old_free_hook = __free_hook;
    __malloc_hook = my_malloc_hook;
    __free_hook = my_free_hook;
}

static inline void *gnu_libc_malloc_hook(std::size_t size, const void *caller)
{
       
}

static inline void gnu_libc__free_hook(void*, const void * caller)
{
    
}


#endif
}//namespace

}//namespace

#endif