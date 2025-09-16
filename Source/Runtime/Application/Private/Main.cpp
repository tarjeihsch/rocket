#include "Main.h"

#include "ConfigFile.h"
#include "Engine.h"

int main(int P_Argc, char* P_Argv[])
{
    RApplicationInfo Info = {};

    if (P_Argc == 2)
    {
        std::string ProjectDirectory = P_Argv[1];

        FConfigFile ProjectConfiguration(ProjectDirectory + "/Project.ini");
        Info.ProjectRootPath = ProjectDirectory;
        Info.ProjectName = ProjectConfiguration.GetData<std::string>("Project", "Name");
        Info.ProjectLibraryPath = ProjectConfiguration.GetData<std::string>("Project", "Path");
        Info.ProjectVersion = ProjectConfiguration.GetData<double>("Project", "Version");
    }

    const auto Engine = new CEngine(Info);

    Engine->Start();

    Engine->Run();

    Engine->Stop();

    delete Engine;

    return 0;
}
