#ifndef _ALIGNED_CONTAINER_POLICY_
#define _ALIGNED_CONTAINER_POLICY_

#include <memory/aligned.hpp>
#include <type_traits>

template<typename T>
class AlignedContainerPolicy
{
    public :
        AlignedContainerPolicy()
        {
            static_assert(std::is_fundamental<T>::value || std::is_base_of<memory::Aligned<>, T>::value, "Allowed concurrent container specialisations : \
                                                                                                         Classes derived from memory::Aligned, fundamental types");
        }
};

#endif