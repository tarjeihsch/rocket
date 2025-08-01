#pragma once

struct ShaderAsset;
class FRHIPipeline;

struct FPipeline
{
    FPipeline(const char* InVertexShaderPath, const char* InFragmentShaderPath);
    ~FPipeline();

    void Compile();
    FRHIPipeline* GetHandle() { return Handle; }

private:
    void Init();
    void Shutdown();

private:
    const char* VertexShaderPath;
    const char* FragmentShaderPath;

    FRHIPipeline *Handle;
};