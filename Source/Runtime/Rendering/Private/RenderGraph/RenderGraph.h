#pragma once

#include "RenderGraphPass.h"
#include "RenderGraphTypes.h"
#include "Common/RHICommandList.h"

class RenderGraphBuilder;

class RenderGraph
{
    RHICommandList command_list;

public:
    RenderGraph();

    TextureHandle ImportTexture(const std::string& name, RHITexture* texture);
    BufferHandle ImportBuffer(const std::string& name, RHIBuffer* buffer);

    TextureHandle ImportTexture(const std::string& name, RHITexture* texture, std::vector<RHISemaphore*> wait_semaphores, std::vector<RHISemaphore*> signal_semaphores);
    BufferHandle ImportBuffer(const std::string& name, RHIBuffer* buffer, std::vector<RHISemaphore*> wait_semaphores, std::vector<RHISemaphore*> signal_semaphores);

    TextureHandle CreateTexture(const std::string& name, RHITextureDesc& texture_desc);
    BufferHandle CreateBuffer(const std::string& name, const RHIBufferDesc& buffer_desc);

    TextureHandle GetTextureHandle(const std::string_view& name) const;
    BufferHandle GetBufferHandle(const std::string_view& name) const;

    RHITexture* ResolveTexture(TextureHandle texture_handle) const;
    RHIBuffer* ResolveBuffer(BufferHandle buffer_handle) const;

    void AddGraphicsPass(const std::string name, PassSetupFunc setup_callback, PassExecuteFunc execute_callback);
    //void AddComputePass(const std::string name, PassSetupFunc setup_callback, PassExecuteFunc execute_callback);

    void Compile();
    void Execute();
    void Reset();

    RHICommandList& GetCommandList();

private:
    std::vector<RenderGraphPass> pass_data;

    std::vector<TextureResource> textures;
    std::vector<BufferResource> buffers;

private:
    friend class RenderGraphBuilder;
    friend class RenderGraphCompiler;
};
