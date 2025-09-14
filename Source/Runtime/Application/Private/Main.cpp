#include "ConfigFile.h"
#include "Engine.h"

#ifdef RK_PLATFORM_LINUX
#include <dlfcn.h>
#endif

#ifdef RK_PLATFORM_WINDOWS
#include <windows.h>
#endif

void* ProjectLibrary = nullptr;

int main(int P_Argc, char* P_Argv[])
{
    if (P_Argc == 2)
    {
        std::string ProjectDirectory = P_Argv[1];

        FConfigFile ProjectConfiguration(ProjectDirectory + "/Project.ini");
        std::string ProjectName = ProjectConfiguration.GetData<std::string>("Project", "Name");
        std::string ProjectPath = ProjectConfiguration.GetData<std::string>("Project", "Path");
        double ProjectVersion = ProjectConfiguration.GetData<double>("Project", "Version");

#if RK_PLATFORM_WINDOWS
        ProjectLibrary = LoadLibraryA((ProjectDirectory + "/" + ProjectPath).c_str());
#endif

#if RK_PLATFORM_LINUX
        ProjectLibrary = dlopen((ProjectDirectory + "/" + ProjectPath).c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
    }

    const auto Engine = new CEngine();

    Engine->Start();

    Engine->Run();

    Engine->Stop();

    delete Engine;

    return 0;
}
