#ifndef __ALIGNED_ALLOCATOR__
#define __ALIGNED_ALLOCATOR__

#include <cstddef>
#include <new>
#include <limits>
#include <stdexcept>

#include <compiler_portability/noexcept.h>
#include "is_power_of_two.h"
#include "cache_line.h"
#include "aligned_memory.h"

namespace memory
{
	
template <typename T, std::size_t alignment = CACHE_LINE_SIZE> 
class AlignedAllocator
{
    public:
        // The following will be the same for virtually all allocators.
		using pointer = T*;
        using const_pointer = const T *;
        using reference = T&;
        using const_reference = const T&;
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        // Default constructor, copy constructor, rebinding constructor, and destructor.
        // Empty for stateless allocators.
        AlignedAllocator()
        { 
            static_assert(is_power_of_two(alignment), "Template argument must be a power of two.");
        } 
        AlignedAllocator(const AlignedAllocator&) { }

        template <typename U> AlignedAllocator(const AlignedAllocator<U>&) { }
        ~AlignedAllocator() { }

        T* address(T& r) const noexcept
        {
            return &r;
        }

        const T * address(const T& s) const noexcept
        {
            return &s;
        }

        size_t max_size() const noexcept
        {
            // The following has been carefully written to be independent of
            // the definition of size_t and to avoid signed/unsigned warnings.
            
            // Extra parantesis are needed for MSVC because Windows headers define min max
            // See http://stackoverflow.com/questions/1904635/warning-c4003-and-errors-c2589-and-c2059-on-x-stdnumeric-limitsintmax
            return (std::numeric_limits<int>::max)() / sizeof(value_type);
        }

        // The following must be the same for all allocators.
        template <typename U> 
        struct rebind 
        {
            typedef AlignedAllocator<U> other;
        };

        bool operator!=(const AlignedAllocator& other) const 
        {
            return !(*this == other);
        }

        void construct(T * const p, const T& t) const 
        {
            void * const pv = static_cast<void *>(p);
            new (pv)T(t);
        }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#endif
        void destroy(T * const p) const
        {
            p->~T();
        }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

        // Returns true if and only if storage allocated from *this
        // can be deallocated from other, and vice versa.
        // Always returns true for stateless allocators.
        bool operator==(const AlignedAllocator& other) const
        {
            return true;
        }

       

        // The following will be different for each allocator.
        T * allocate(const std::size_t n) const 
        {

            // The return value of allocate(0) is unspecified.
            // AlignedAllocator returns NULL in order to avoid depending
            // on malloc(0)’s implementation-defined behavior
            // (the implementation can define malloc(0) to return NULL,
            // in which case the bad_alloc check below would fire).
            // All allocators can return NULL in this case.
            if (n == 0) 
            {
                return nullptr;
            }

            // All allocators should contain an integer overflow check.
            // The Standardization Committee recommends that std::length_error
            // be thrown in the case of integer overflow.
            if (n > max_size()) 
            {
                throw std::length_error("AlignedAllocator<T>::allocate() – Integer overflow.");
            }

            void * const pv = alignedMalloc(n * sizeof(T), alignment);

            // Allocators should throw std::bad_alloc in the case of memory allocation failure.
            if (pv == nullptr) 
            {
                throw std::bad_alloc();
            }

            return static_cast<T *>(pv);
        }

        void deallocate(T * const p, const size_t n) const 
        {
            alignedFree(p);
        }

        // The following will be the same for all allocators that ignore hints.
        template <typename U> T * allocate(const size_t n, const U * /* const hint */) const {
            return allocate(n);
        }

        // NOTE FOR MSVC : 
        // Allocators are not required to be assignable, so
        // all allocators should have a private unimplemented
        // assignment operator. Note that this will trigger the
        // off-by-default (enabled under /Wall) warning C4626
        // “assignment operator could not be generated because a
        // base class assignment operator is inaccessible” within
        // the STL headers, but that warning is useless.

    private:

        AlignedAllocator& operator=(const AlignedAllocator&);

    };

} // namespace

#endif