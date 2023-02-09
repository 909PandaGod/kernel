#ifndef __NOSTDLIB_BITS_DECLVAL_H__
#define __NOSTDLIB_BITS_DECLVAL_H__
#include "type_traits/ref.h"

namespace std
{
    template <typename T>
    std::add_lvalue_reference_t<T> declval() noexcept { __builtin_unreachable(); }
} // namespace std

#endif