#pragma once

#include <cstdint>

using ANSICHAR = char;
using WCHAR = wchar_t;

constexpr static int32_t INT_MX = 0x7FFFFFFF;

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   (__builtin_expect(!!(x), 1))
    #define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif