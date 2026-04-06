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

#define DECLARE_DELEGATE(name, return_type) typedef return_type(*name)()
#define DECLARE_DELEGATE_OneParam(name, return_type, param_1) typedef return_type(*name)(param_1)
#define DECLARE_DELEGATE_TwoParams(name, return_type, param_1, param_2) typedef return_type(*name)(param_1, param_2)
#define DECLARE_DELEGATE_ThreeParams(name, return_type, param_1, param_2, param_3) typedef return_type(*name)(param_1, param_2, param_3)