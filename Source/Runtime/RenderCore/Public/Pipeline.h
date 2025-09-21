#pragma once
#include "Common/RHIPipeline.h"

#include <cstdint>
#include <memory>

struct FRHIBuffer;
class FRHIPipeline;

struct FPipeline
{
    FPipeline(const char *InVertexShaderPath, const char *InFragmentShaderPath);
    FPipeline(FRHIPipelineDesc PipelineDesc);
    ~FPipeline();

    FRHIPipeline *GetHandle()
    {
        return Handle;
    }

private:
    void Init();
    void Shutdown();

private:
    const char *VertexShaderPath;
    const char *FragmentShaderPath;

    FRHIPipeline *Handle;
};