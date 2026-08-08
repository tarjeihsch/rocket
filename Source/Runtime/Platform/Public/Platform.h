#pragma once

#include <cstdint>
#include <vector>
#include <string>

#if defined(_WIN32)
    #include <Windows.h>
#elif defined(__APPLE__)
    #include <pthread.h>
#elif defined(__linux__)
    #include <pthread.h>
#endif

#include "Application.h"

namespace Platform
{
    void SetThreadName(const char* P_Thread_Name);
}
