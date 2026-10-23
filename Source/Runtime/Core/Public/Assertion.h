#pragma once

#include <Log.h>

// Evaluated in every configuration so a failure is always logged with its
// location, even on machines without a debugger. Debug builds also break.
//
// An optional printf-style message can follow the condition:
//     ensure(index < count, "Index %u out of range (count %u)", index, count);

#if defined(_MSC_VER)
    #define DEBUG_BREAK() __debugbreak()
#else
    #define DEBUG_BREAK() __builtin_trap()
#endif

#ifndef NDEBUG
    #define ENSURE_BREAK() DEBUG_BREAK()
#else
    #define ENSURE_BREAK() ((void)0)
#endif

#define ensure(condition, ...) \
    do \
    { \
        if (!(condition)) \
        { \
            Log::EnsureFailed(#condition, __FILE__, __LINE__, ##__VA_ARGS__); \
            ENSURE_BREAK(); \
        } \
    } while (0)
