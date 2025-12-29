#include "FrameGraph.h"

#include "Logger.h"
#include "Common/RHISurface.h"
#include "Common/RHITexture.h"
#include "Common/RHIBuffer.h"
#include "Common/RHICommandList.h"

#include <queue>
#include <Common/RHISemaphore.h>

static void AddEdge(FGPass& From, FGPass& To, uint32_t FromIndex, uint32_t ToIndex, ERGResourceKind Kind, uint32_t ResourceId, FDGEdgeReason::EType Type, FrameDependencyGraph& OutFDG)
{
    From.OutEdges.push_back(ToIndex);
    To.InDegree++;

    OutFDG.Edges.push_back({ FromIndex, ToIndex });

    FDGEdgeReason Reason = {};
    Reason.FromPass = FromIndex;
    Reason.ToPass = ToIndex;
    Reason.Kind = Kind;
    Reason.ResourceId = ResourceId;
    Reason.Type = Type;
    OutFDG.EdgeReasons.push_back(Reason);
}

static const char* HazardToString(FDGEdgeReason::EType Type)
{
    switch (Type)
    {
        case FDGEdgeReason::EType::ReadAfterWrite:  return "RAW";
        case FDGEdgeReason::EType::WriteAfterWrite: return "WAW";
        default: return "UNK";
    }
}

FrameGraphBuilder::FrameGraphBuilder(FrameGraph& InGraph)
    : Graph(InGraph)
{
}

FGTextureHandle FrameGraphBuilder::CreateTexture(std::string_view Name, const FRHITextureDesc& Desc)
{
    return Graph.AllocTexture(Name, Desc);
}

FGTextureViewHandle FrameGraphBuilder::CreateTextureView(std::string_view Name, const FRHITextureViewDesc& Desc, FGTextureHandle Texture)
{
    return Graph.AllocTextureView(Name, Desc, Texture);
}

FRGBufferHandle FrameGraphBuilder::CreateBuffer(std::string_view Name, const FRHIBufferDesc& Desc)
{
    return Graph.AllocBuffer(Name, Desc);
}

FGTextureHandle FrameGraphBuilder::RegisterExternalTexture(std::string_view Name, RHITexture* Texture, FRHITextureView* DefaultView, const FGTextureState& InitialState, const FGTextureState& FinalState)
{
    return Graph.AllocExternalTexture(Name, Texture, DefaultView, InitialState, FinalState);
}

FRGBufferHandle FrameGraphBuilder::RegisterExternalBuffer(std::string_view Name, FRHIBuffer* Buffer, const FRGBufferState& InitialState, const FRGBufferState& FinalState)
{
    return Graph.AllocExternalBuffer(Name, Buffer, InitialState, FinalState);
}

uint32_t FrameGraphBuilder::AddPass(std::string_view Name, const std::function<void(FrameGraphBuilder& Builder, FGPass& Pass)>& Setup, const std::function<void(IRHICommandListContext& CmdListCtx)>& Execute)
{
    uint32_t PassIndex = Graph.AllocPass(Name, Execute);
    Setup(*this, Graph.Passes[PassIndex]);
    return PassIndex;
}

void FrameGraphBuilder::ReadTexture(FGPass& Pass, FGTextureHandle Texture, ERGTextureUsage Usage)
{
    ensure(Texture.IsValid(), "Invalid texture handle");
    Pass.Textures.push_back({ Texture, Usage, ERGAccess::Read });
}

void FrameGraphBuilder::WriteTexture(FGPass& Pass, FGTextureHandle Texture, ERGTextureUsage Usage)
{
    ensure(Texture.IsValid(), "Invalid texture handle");
    Pass.Textures.push_back({ Texture, Usage, ERGAccess::Write });
}

void FrameGraphBuilder::ReadBuffer(FGPass& Pass, FRGBufferHandle Buffer, ERGBufferUsage Usage, uint64_t Offset, uint64_t Size)
{
    ensure(Buffer.IsValid(), "Invalid buffer handle");
    Pass.Buffers.push_back({ Buffer, Usage, ERGAccess::Read, Offset, Size });
}

void FrameGraphBuilder::WriteBuffer(FGPass& Pass, FRGBufferHandle Buffer, ERGBufferUsage Usage, uint64_t Offset, uint64_t Size)
{
    ensure(Buffer.IsValid(), "Invalid buffer handle");
    Pass.Buffers.push_back({ Buffer, Usage, ERGAccess::Write, Offset, Size });
}

RHITexture* FrameGraphBuilder::ResolveTexture(FGTextureHandle Texture) const
{
    return Graph.ResolveTexture(Texture);
}

FRHITextureView* FrameGraphBuilder::ResolveTextureView(FGTextureViewHandle View) const
{
    return Graph.ResolveTextureView(View);
}

FRHIBuffer* FrameGraphBuilder::ResolveBuffer(FRGBufferHandle Buffer) const
{
    return Graph.ResolveBuffer(Buffer);
}

void FrameGraph::Reset()
{
    for (FRGTextureView& View : Views)
    {
        if (!View.bExternal && View.Physical)
        {
            delete View.Physical;
            View.Physical = nullptr;
        }
    }

    for (FRGTexture& Texture : Textures)
    {
        if (!Texture.bExternal && Texture.Physical)
        {
            delete Texture.Physical;
            Texture.Physical = nullptr;
        }
    }

    for (FRGBuffer& Buffer : Buffers)
    {
        if (!Buffer.bExternal && Buffer.Physical)
        {
            delete Buffer.Physical;
            Buffer.Physical = nullptr;
        }
    }

    Textures.clear();
    Views.clear();
    Buffers.clear();

    ExternalTextures.clear();
    ExternalBuffers.clear();

    Passes.clear();
    CompiledPasses.clear();
}

FrameGraphBuilder FrameGraph::Builder()
{
    return FrameGraphBuilder(*this);
}

FGTextureHandle FrameGraph::AllocTexture(std::string_view Name, const FRHITextureDesc& Desc)
{
    FRGTexture Tex;
    Tex.Name = std::string(Name);
    Tex.Desc = Desc;
    Tex.State = { Desc.InitialLayout, ERHIResourceAccess::None, ERHIPipelineStage::TopOfPipe };
    Tex.bExternal = false;

    uint32_t Id = static_cast<uint32_t>(Textures.size());
    Textures.push_back(std::move(Tex));
    return { Id };
}

FGTextureViewHandle FrameGraph::AllocTextureView(std::string_view Name, const FRHITextureViewDesc& Desc, FGTextureHandle Texture)
{
    FRGTextureView View;
    View.Name = std::string(Name);
    View.Desc = Desc;
    View.Texture = Texture;
    View.Physical = nullptr;
    View.bExternal = false;

    uint32_t Id = static_cast<uint32_t>(Views.size());
    Views.push_back(std::move(View));
    return { Id };
}

FGTextureHandle FrameGraph::AllocExternalTexture(std::string_view Name, RHITexture* Texture, FRHITextureView* DefaultView, const FGTextureState& InitialState, const FGTextureState& FinalState)
{
    ensure(Texture != nullptr, "External texture cannot be null");

    FRGExternalTexture Ext;
    Ext.Texture = Texture;
    Ext.DefaultView = DefaultView;
    Ext.InitialState = InitialState;
    Ext.FinalState = FinalState;
    ExternalTextures.push_back(Ext);

    FRGTexture Tex;
    Tex.Name = std::string(Name);
    Tex.Desc = Texture->RHIGetTextureDesc();
    Tex.Physical = Texture;
    Tex.State = InitialState;
    Tex.bExternal = true;

    uint32_t Id = static_cast<uint32_t>(Textures.size());
    Textures.push_back(std::move(Tex));

    if (DefaultView)
    {
        FRGTextureView View;
        View.Desc = DefaultView->RHIGetTextureViewDesc();
        View.Texture = { Id };
        View.Physical = DefaultView;
        View.bExternal = true;
        Views.push_back(std::move(View));
    }

    return { Id };
}

FRGBufferHandle FrameGraph::AllocBuffer(std::string_view Name, const FRHIBufferDesc& Desc)
{
    FRGBuffer Buffer = {};
    Buffer.Name = Name;
    Buffer.Desc = Desc;
    Buffer.Physical = nullptr;
    Buffer.bExternal = false;

    uint32_t Id = static_cast<uint32_t>(Buffers.size());
    Buffers.push_back(std::move(Buffer));
    return { Id };
}

FRGBufferHandle FrameGraph::AllocExternalBuffer(std::string_view Name, FRHIBuffer* InBuffer, const FRGBufferState& InitialState, const FRGBufferState& FinalState)
{
    ensure(InBuffer != nullptr);

    FRGExternalBuffer ExternalBuffer = {};
    ExternalBuffer.Buffer = InBuffer;
    ExternalBuffer.InitialState = InitialState;
    ExternalBuffer.FinalState = FinalState;
    ExternalBuffers.push_back(ExternalBuffer);

    FRGBuffer Buffer = {};
    Buffer.Name = std::string(Name);
    Buffer.Desc = InBuffer->GetBufferDesc();
    Buffer.Physical = InBuffer;
    Buffer.bExternal = true;

    uint32_t Id = static_cast<uint32_t>(Buffers.size());
    Buffers.push_back(std::move(Buffer));
    return { Id };
}

uint32_t FrameGraph::AllocPass(std::string_view Name, const std::function<void(IRHICommandListContext&)>& Execute)
{
    FGPass Pass;
    Pass.Name = std::string(Name);
    Pass.Execute = Execute;

    uint32_t Id = static_cast<uint32_t>(Passes.size());
    Passes.push_back(std::move(Pass));
    return Id;
}

RHITexture* FrameGraph::ResolveTexture(FGTextureHandle Texture) const
{
    ensure(Texture.IsValid() && Texture.Id < Textures.size(), "Invalid texture handle");
    return Textures[Texture.Id].Physical;
}

FRHITextureView* FrameGraph::ResolveTextureView(FGTextureViewHandle View) const
{
    ensure(View.IsValid() && View.Id < Views.size(), "Invalid view handle");
    return Views[View.Id].Physical;
}

FRHIBuffer* FrameGraph::ResolveBuffer(FRGBufferHandle Buffer) const
{
    ensure(Buffer.IsValid() && Buffer.Id < Buffers.size(), "Invalid buffer handle");
    return Buffers[Buffer.Id].Physical;
}

void FrameGraph::compile()
{
    FrameDependencyGraph FDG;

    for (auto& Pass : Passes)
    {
        Pass.OutEdges.clear();
        Pass.InDegree = 0;
    }

    FDG.Nodes.clear();
    FDG.Edges.clear();

    FDG.Nodes.reserve(Passes.size());
    for (const auto& Pass : Passes)
    {
        FDG.Nodes.push_back(Pass.Name);
    }

    std::unordered_map<uint32_t, uint32_t> LastWriter;
    LastWriter.reserve(Textures.size());

    for (uint32_t PassIndex = 0; PassIndex < static_cast<uint32_t>(Passes.size()); ++PassIndex)
    {
        FGPass& Pass = Passes[PassIndex];

        for (const FRGTextureUse& Use : Pass.Textures)
        {
            if (!Use.Texture.IsValid())
            {
                continue;
            }

            if (Use.Access == ERGAccess::Read)
            {
                auto It = LastWriter.find(Use.Texture.Id);
                if (It != LastWriter.end())
                {
                    uint32_t WriterPass = It->second;
                    if (WriterPass != PassIndex)
                    {
                        AddEdge(Passes[WriterPass], Pass, WriterPass, PassIndex, ERGResourceKind::Texture, Use.Texture.Id, FDGEdgeReason::EType::ReadAfterWrite, FDG);
                    }
                }
            }
        }

        for (const FRGTextureUse& Use : Pass.Textures)
        {
            if (!Use.Texture.IsValid())
            {
                continue;
            }

            if (Use.Access == ERGAccess::Write)
            {
                auto It = LastWriter.find(Use.Texture.Id);
                if (It != LastWriter.end())
                {
                    uint32_t PrevWriter = It->second;
                    if (PrevWriter != PassIndex)
                    {
                        AddEdge(Passes[PrevWriter], Pass, PrevWriter, PassIndex, ERGResourceKind::Texture, Use.Texture.Id, FDGEdgeReason::EType::WriteAfterWrite, FDG);
                    }
                }

                LastWriter[Use.Texture.Id] = PassIndex;
            }
        }
    }

    std::unordered_map<uint32_t, uint32_t> LastBufferWriter;
    LastBufferWriter.reserve(Buffers.size());

    for (uint32_t PassIndex = 0; PassIndex < (uint32_t)Passes.size(); ++PassIndex)
    {
        FGPass& Pass = Passes[PassIndex];

        for (const FRGBufferUse& Use : Pass.Buffers)
        {
            if (Use.Access == ERGAccess::Read)
            {
                auto It = LastBufferWriter.find(Use.Buffer.Id);
                if (It != LastBufferWriter.end() && It->second != PassIndex)
                {
                    AddEdge(Passes[It->second], Pass, It->second, PassIndex, ERGResourceKind::Buffer, Use.Buffer.Id, FDGEdgeReason::EType::ReadAfterWrite, FDG);
                }
            }
        }

        for (const FRGBufferUse& Use : Pass.Buffers)
        {
            if (Use.Access == ERGAccess::Write)
            {
                auto It = LastBufferWriter.find(Use.Buffer.Id);
                if (It != LastBufferWriter.end() && It->second != PassIndex)
                {
                    AddEdge(Passes[It->second], Pass, It->second, PassIndex, ERGResourceKind::Buffer, Use.Buffer.Id, FDGEdgeReason::EType::WriteAfterWrite, FDG);
                }
                LastBufferWriter[Use.Buffer.Id] = PassIndex;
            }
        }
    }

    for (FRGTexture& Texture : Textures)
    {
        if (Texture.bExternal)
        {
            ensure(Texture.Physical != nullptr, "External texture must have a physical handle");
            continue;
        }

        if (!Texture.Physical)
        {
            Texture.Physical = GetRHI()->RHICreateTexture(Texture.Desc);
        }
    }

    for (FRGTextureView& View : Views)
    {
        if (View.bExternal)
        {
            ensure(View.Physical != nullptr, "External view must have a physical handle");
            continue;
        }

        if (!View.Physical)
        {
            RHITexture* Texture = ResolveTexture(View.Texture);
            View.Physical = GetRHI()->RHICreateTextureView(View.Desc, Texture);
        }
    }

    for (FRGBuffer& Buffer : Buffers)
    {
        if (Buffer.bExternal)
        {
            ensure(Buffer.Physical != nullptr, "External buffer must have a physical handle");
            continue;
        }

        if (!Buffer.Physical)
        {
            Buffer.Physical = GetRHI()->RHICreateBuffer(Buffer.Desc);
        }
    }
    // Topological sort (Kahn)
    std::queue<uint32_t> Ready;
    for (uint32_t Index = 0; Index < static_cast<uint32_t>(Passes.size()); ++Index)
    {
        if (Passes[Index].InDegree == 0)
        {
            Ready.push(Index);
        }
    }

    CompiledPasses.clear();
    CompiledPasses.reserve(Passes.size());

    while (!Ready.empty())
    {
        uint32_t Node = Ready.front();
        Ready.pop();
        CompiledPasses.push_back(Node);

        for (uint32_t Next : Passes[Node].OutEdges)
        {
            Passes[Next].InDegree -= 1;

            if (Passes[Next].InDegree == 0)
            {
                Ready.push(Next);
            }
        }
    }

    ensure(CompiledPasses.size() == Passes.size(), "Frame graph has a cycle (cannot compile)");
    FDG.TopologicalOrder = CompiledPasses;
}

void FrameGraph::Barrier(IRHICommandListContext& CmdListCtx, const FGPass& Pass)
{
    for (const FRGTextureUse& Use : Pass.Textures)
    {
        FRGTexture& RGTexture = Textures[Use.Texture.Id];
        RHITexture* Texture = RGTexture.Physical;

        FGTextureState NewTextureState = GetTextureState(Use.Usage, Use.Access);

        if (Texture->current_layout != NewTextureState.Layout || Texture->current_access != NewTextureState.Access || Texture->current_pipeline_stage != NewTextureState.Stage)
        {
            CmdListCtx.RHIPipelineBarrier(Texture, Texture->current_layout, NewTextureState.Layout, Texture->current_access, NewTextureState.Access, Texture->current_pipeline_stage, NewTextureState.Stage, Texture->CurrentQueueFamilyIndex, Texture->CurrentQueueFamilyIndex);
        }
    }

    for (const FRGBufferUse& Use : Pass.Buffers)
    {
        FRGBuffer& RGBuffer = Buffers[Use.Buffer.Id];
        FRHIBuffer* Buffer = RGBuffer.Physical;

        FRGBufferState NewBufferState = GetBufferState(Use.Usage, Use.Access);

        if (Buffer->CurrentAccess != NewBufferState.Access || Buffer->CurrentPipelineStage != NewBufferState.Stage || Buffer->CurrentQueueFamilyIndex != NewBufferState.QueueFamilyIndex)
        {
            CmdListCtx.RHIPipelineBarrier(Buffer, Buffer->CurrentAccess, NewBufferState.Access, Buffer->CurrentPipelineStage, NewBufferState.Stage, Buffer->CurrentQueueFamilyIndex, NewBufferState.QueueFamilyIndex, Use.Offset, Use.Size == ~0ULL ? (~0ULL) : Use.Size);
        }
    }
}

void FrameGraph::execute(IRHICommandListContext& ctx)
{
    for (uint32_t PassIndex : CompiledPasses)
    {
        const FGPass& Pass = Passes[PassIndex];
        Barrier(ctx, Pass);
        Pass.Execute(ctx);
    }

    for (FRGTexture& RGTexture : Textures)
    {
        if (!RGTexture.bExternal)
        {
            continue;
        }

        RHITexture* Texture = RGTexture.Physical;
        ensure(Texture != nullptr, "External texture must have a physical handle");

        for (const FRGExternalTexture& ExternalTexture : ExternalTextures)
        {
            if (ExternalTexture.Texture != Texture)
            {
                continue;
            }

            const FGTextureState& Final = ExternalTexture.FinalState;

            if (Texture->current_layout != Final.Layout || Texture->current_access != Final.Access || Texture->current_pipeline_stage != Final.Stage)
            {
                ctx.RHIPipelineBarrier(Texture, Texture->current_layout, Final.Layout, Texture->current_access, Final.Access, Texture->current_pipeline_stage, Final.Stage, Texture->CurrentQueueFamilyIndex, Texture->CurrentQueueFamilyIndex);
            }

            break;
        }
    }
}

FGTextureState FrameGraph::GetTextureState(ERGTextureUsage Usage, ERGAccess Access)
{
    FGTextureState State = {};
    State.Access = ERHIResourceAccess::None;
    State.Stage = ERHIPipelineStage::None;
    State.Layout = ERHITextureLayout::Undefined;

    switch (Usage)
    {
        case ERGTextureUsage::ColorAttachment:
        {
            State.Layout = ERHITextureLayout::ColorAttachment;
            State.Access = ERHIResourceAccess::ColorAttachmentWrite;
            State.Stage = ERHIPipelineStage::ColorAttachment;
            break;
        }

        case ERGTextureUsage::DepthStencilAttachment:
        {
            State.Layout = ERHITextureLayout::DepthStencil;
            State.Access = ERHIResourceAccess::DepthStencilAttachmentWrite;
            State.Stage = ERHIPipelineStage::DepthStencil;
            break;
        }

        case ERGTextureUsage::ShaderRead:
        {
            State.Layout = ERHITextureLayout::ShaderReadOnly;
            State.Access = ERHIResourceAccess::ShaderSampledRead;
            State.Stage = ERHIPipelineStage::FragmentShader;
            break;
        }

        case ERGTextureUsage::ShaderWrite:
        {
            State.Layout = ERHITextureLayout::General;
            State.Access = ERHIResourceAccess::ShaderStorageWrite;
            State.Stage = ERHIPipelineStage::ComputeShader;
            break;
        }

        case ERGTextureUsage::CopySrc:
        {
            State.Layout = ERHITextureLayout::TransferSrc;
            State.Access = ERHIResourceAccess::TransferRead;
            State.Stage = ERHIPipelineStage::Transfer;
            break;
        }

        case ERGTextureUsage::CopyDst:
        {
            State.Layout = ERHITextureLayout::TransferDst;
            State.Access = ERHIResourceAccess::TransferWrite;
            State.Stage = ERHIPipelineStage::Transfer;
            break;
        }

        case ERGTextureUsage::Present:
        {
            State.Layout = ERHITextureLayout::Present;
            State.Access = ERHIResourceAccess::None;
            State.Stage = ERHIPipelineStage::BottomOfPipe;
            break;
        }

        default:
        {
            State.Layout = ERHITextureLayout::Undefined;
            State.Access = ERHIResourceAccess::None;
            State.Stage = ERHIPipelineStage::TopOfPipe;
            break;
        }
    }

    if (Access == ERGAccess::Write && Usage == ERGTextureUsage::ShaderRead)
    {
        State.Layout = ERHITextureLayout::General;
        State.Access = ERHIResourceAccess::ShaderStorageWrite;
        State.Stage = ERHIPipelineStage::ComputeShader;
    }

    if (Access == ERGAccess::Read && Usage == ERGTextureUsage::ShaderWrite)
    {
        State.Layout = ERHITextureLayout::General;
        State.Access = ERHIResourceAccess::ShaderStorageRead;
        State.Stage = ERHIPipelineStage::ComputeShader;
    }

    return State;
}

FRGBufferState FrameGraph::GetBufferState(ERGBufferUsage Usage, ERGAccess Access)
{
    FRGBufferState BufferState = {};
    BufferState.QueueFamilyIndex = ~(0U);

    switch (Usage)
    {
        case ERGBufferUsage::Vertex:
        {
            BufferState.Access = ERHIResourceAccess::None;
            BufferState.Stage = ERHIPipelineStage::VertexShader;
            break;
        }

        case ERGBufferUsage::Index:
        {
            BufferState.Access = ERHIResourceAccess::None;
            BufferState.Stage = ERHIPipelineStage::VertexShader;
            break;
        }

        case ERGBufferUsage::Uniform:
        {
            BufferState.Access = ERHIResourceAccess::ShaderSampledRead;
            BufferState.Stage = ERHIPipelineStage::VertexShader | ERHIPipelineStage::FragmentShader;
            break;
        }

        case ERGBufferUsage::StorageRead:
        {
            BufferState.Access = ERHIResourceAccess::ShaderStorageRead;
            BufferState.Stage = ERHIPipelineStage::ComputeShader | ERHIPipelineStage::FragmentShader;
            break;
        }

        case ERGBufferUsage::StorageWrite:
        {
            BufferState.Access = ERHIResourceAccess::ShaderStorageWrite;
            BufferState.Stage = ERHIPipelineStage::ComputeShader;
            break;
        }

        case ERGBufferUsage::Indirect:
        {
            BufferState.Access = ERHIResourceAccess::None;
            BufferState.Stage = ERHIPipelineStage::VertexShader;
            break;
        }

        case ERGBufferUsage::CopySrc:
        {
            BufferState.Access = ERHIResourceAccess::TransferRead;
            BufferState.Stage = ERHIPipelineStage::Transfer;
            break;
        }

        case ERGBufferUsage::CopyDst:
        {
            BufferState.Access = ERHIResourceAccess::TransferWrite;
            BufferState.Stage = ERHIPipelineStage::Transfer;
            break;
        }

        default:
        {
            BufferState.Access = ERHIResourceAccess::None;
            BufferState.Stage = ERHIPipelineStage::TopOfPipe;
            break;
        }
    }

    if (Access == ERGAccess::Write && (Usage == ERGBufferUsage::StorageRead || Usage == ERGBufferUsage::Uniform))
    {
        BufferState.Access = ERHIResourceAccess::ShaderStorageWrite;
        BufferState.Stage = ERHIPipelineStage::ComputeShader;
    }

    return BufferState;
}