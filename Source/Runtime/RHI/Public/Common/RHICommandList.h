#pragma once

#include "RHITexture.h"

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

struct FRHIDrawCommand;
struct FRHIDrawParams;
struct FRHIBuffer;
struct FRHIPipeline;
struct FRHITexture;
struct FRHITextureView;
struct FMeshDrawCommand;
enum class ERHITextureLayout : uint8_t;
enum class ERHIResourceAccess : uint16_t;
enum class ERHIPipelineStage : uint16_t;

class IRHICommandListContext
{
public:
    virtual ~IRHICommandListContext() = default;

    virtual IRHICommandListContext& GetLowestLevelContext() { return *this; }

    virtual void RHIBindPipeline(FRHIPipeline* Pipeline) = 0;
    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) = 0;
    virtual void RHIDraw(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) = 0;
    virtual void RHIDrawIndexed(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& InDrawParams) = 0;
    virtual void RHIPipelineBarrier(FRHITexture* InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex = ~(0U), uint32_t InDstQueueFamilyIndex= ~(0U)) = 0;
    virtual void RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachment) = 0;
    virtual void RHIEndRendering() = 0;
    virtual void RHICopyBuffer(FRHIBuffer* Src, FRHIBuffer* Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize) = 0;
    virtual void RHICmdBlitImage(FRHITexture* Src, FRHITexture* Dst) = 0;
};

struct FRHICommandBase
{
    using TFunc = void(*)(FRHICommandBase*, IRHICommandListContext&);

    TFunc ExecuteAndDestruct = nullptr;
    uint32_t ByteSize = 0;

    void Dispatch(IRHICommandListContext& Context)
    {
        ExecuteAndDestruct(this, Context);
    }
};

template<typename TDerived>
struct TRHICommand : FRHICommandBase
{
    TRHICommand()
    {
        ByteSize = sizeof(TDerived);
        ExecuteAndDestruct = [](FRHICommandBase* Base, IRHICommandListContext& Context)
        {
            auto* Self = static_cast<TDerived*>(Base);
            Self->Execute_Internal(Context);
            Self->~TDerived();
        };
    }
};

class FRHICommandList
{
public:
    explicit FRHICommandList(IRHICommandListContext& InContext)
        : Context(&InContext)
    {
    }

    inline IRHICommandListContext& GetContext() const
    {
        return *Context;
    }

    template<typename TCommand, typename... TArgs>
    void Enqueue(TArgs&&... Args)
    {
        size_t Offset = Buffer.size();
        Buffer.resize(Offset + sizeof(TCommand));
        new (Buffer.data() + Offset) TCommand(std::forward<TArgs>(Args)...);
    }

    std::vector<uint8_t> Flush()
    {
        std::vector<uint8_t> Temp;
        Temp.swap(Buffer);
        return Temp;
    }

private:
    IRHICommandListContext* Context;
    std::vector<uint8_t> Buffer;
};

class FRHICommandListImmediate : public FRHICommandList
{
public:
    explicit FRHICommandListImmediate(IRHICommandListContext& Ctx)
        : FRHICommandList(Ctx), Context(Ctx)
    {
    }

    template<class TCommand, class... TArgs>
    void EnqueueImmediate(TArgs&&... Args)
    {
        TCommand Command(std::forward<TArgs>(Args)...);
        Command.Execute_Internal(Context);
    }

private:
    IRHICommandListContext& Context;
};

class FRHICommandListExecutor
{
public:
    static void Execute(IRHICommandListContext& Context, std::vector<uint8_t>&& Buffer)
    {
        uint8_t* Pointer = Buffer.data();
        uint8_t* EndPointer = Pointer + Buffer.size();

        while (Pointer < EndPointer)
        {
            auto* Command = reinterpret_cast<FRHICommandBase*>(Pointer);
            Command->Dispatch(Context);
            Pointer += Command->ByteSize;
        }
    }
};
