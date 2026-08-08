#pragma once

#include <cstdint>

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using ANSICHAR = char;
using WCHAR = wchar_t;

constexpr static int32_t INTEGER_MAX  = 0x7FFFFFFF;

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