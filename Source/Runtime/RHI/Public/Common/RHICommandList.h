#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

class RHISwapchain;
struct RHISemaphore;
enum class ERHIQueueSyncStage : uint8_t;
struct RRHIRect;
struct FRHIDrawCommand;
struct FRHIDrawParams;
struct FRHIDrawCall;
struct FRHIBuffer;
struct FRHIPipeline;
struct RHITexture;
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
    virtual void RHISetScissorRect(int32_t X, int32_t Y, uint32_t Width, uint32_t Height) = 0;
    virtual void RHIDraw(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) = 0;
    virtual void RHIDrawIndexed(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& InDrawParams) = 0;
    virtual void RHIDrawIndexed2(const FRHIDrawCall& DrawCall) = 0;
    virtual void RHIPipelineBarrier(RHITexture* InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex) = 0;
    virtual void RHIPipelineBarrier(FRHIBuffer* InBuffer, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex, uint64_t InOffset, uint64_t InSize) = 0;
    virtual void RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachment, bool bClearColor) = 0;
    virtual void RHIEndRendering() = 0;
    virtual void RHICopyBuffer(FRHIBuffer* Src, FRHIBuffer* Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize) = 0;
    virtual void RHICmdBlitImage(RHITexture* Src, RHITexture* Dst) = 0;
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

class RHICommandList
{
public:
    explicit RHICommandList(IRHICommandListContext& InContext)
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

class FRHICommandListImmediate : public RHICommandList
{
public:
    explicit FRHICommandListImmediate(IRHICommandListContext& Ctx)
        : RHICommandList(Ctx), Context(Ctx)
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

class RHICommandListExecutor
{
public:
    static void execute(IRHICommandListContext& Context, std::vector<uint8_t>&& Buffer)
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
