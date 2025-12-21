#include "IRByteCode.h"

#include "Assertion.h"

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #include <dxc/dxcapi.h>
#endif

#ifdef RK_PLATFORM_WINDOWS
    #include <windows.h>
    #include <dxcapi.h>
    #include <wrl/client.h>
    using Microsoft::WRL::ComPtr;
#endif

IRByteCode AssetLoader<IRByteCode, IRByteCodeLoaderOptions>::Load(const AssetLoaderRequest& loaderRequest, const IRByteCodeLoaderOptions& loaderOptions)
{
    IRByteCode Result;

#ifdef RK_PLATFORM_LINUX

#endif

#ifdef RK_PLATFORM_WINDOWS

#endif
    return Result;
}
