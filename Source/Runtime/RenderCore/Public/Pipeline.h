#pragma once
#include <cstdint>
#include <memory>

struct FRHIBuffer;
class FRHIPipeline;

struct FPipeline
{
    FPipeline(const char *InVertexShaderPath, const char *InFragmentShaderPath);
    ~FPipeline();

    void Compile();
    FRHIPipeline *GetHandle()
    {
        return Handle;
    }

    template <typename T>
    std::shared_ptr<FRHIBuffer> BindBuffer(T *Data);
    std::shared_ptr<FRHIBuffer> BindBuffer(void *Data, uint64_t Size);

    // void BindTexture();

private:
    void Init();
    void Shutdown();

private:
    const char *VertexShaderPath;
    const char *FragmentShaderPath;

    FRHIPipeline *Handle;
};

template <typename T>
std::shared_ptr<FRHIBuffer>  FPipeline::BindBuffer(T *Data)
{
    return BindBuffer(Data, sizeof(T));
}