#include "Module.h"

#include "Assert.h"

#ifdef RK_PLATFORM_LINUX
#include <dlfcn.h>
#endif

#ifdef RK_PLATFORM_WINDOWS
#include <windows.h>
#endif

RModule::RModule(const char* InPath)
    : Path(InPath)
{
    Load();
}

RModule::~RModule()
{
    Release();
}

void* RModule::GetSymbol(const char *Name)
{
#if RK_PLATFORM_WINDOWS
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(Library), Name));
#endif

#if RK_PLATFORM_LINUX
    return dlsym(Library, Name);
#endif

    RK_ASSERT(false, "Failed to find symbol from given name");
}

void RModule::Load()
{
#if RK_PLATFORM_WINDOWS
    Library = LoadLibraryA(Path);
#endif

#if RK_PLATFORM_LINUX
    Library = dlopen(Path, RTLD_NOW | RTLD_GLOBAL);
#endif

    RK_ASSERT(Library, "Failed to load library from given path");

    Generation += 1;
}

void RModule::Release()
{
#if RK_PLATFORM_WINDOWS
    FreeLibrary(reinterpret_cast<HMODULE>(Library));
#endif

#if RK_PLATFORM_LINUX
    dlclose(Library);
#endif

    Library = nullptr;
}

