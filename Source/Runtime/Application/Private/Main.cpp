#include "Engine.h"

int main()
{
    const auto Engine = new CEngine();

    Engine->Start();

    Engine->Run();

    Engine->Stop();

    delete Engine;
    
    return 0;
}
