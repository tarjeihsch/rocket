#include "Platform.h"

void Platform::SetThreadName(const char* P_Thread_Name)
{
#if PLATFORM_WINDOWS
        std::wstring Wide(P_Name.begin(), P_Name.end());
        SetThreadDescription(GetCurrentThread(), Wide.c_str());
#elif PLATFORM_APPLE
        pthread_setname_np(P_Name.c_str());
#elif PLATFORM_LINUX
        pthread_setname_np(pthread_self(), P_Name.substr(0, 15).c_str());
#endif
}
