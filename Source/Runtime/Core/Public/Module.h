#pragma once
#include <cstdint>

class RModule
{
public:
    explicit RModule(const char* Path);
    RModule(const RModule&) = delete;
    RModule& operator=(const RModule&) = delete;

    ~RModule();

    void* GetSymbol(const char* Name);

private:
    void Load();
    void Release();

private:
    void* Library;

    const char* Path;

    uint16_t Generation;
};

extern RModule* GScriptModule;