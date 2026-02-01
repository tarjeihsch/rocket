#include "RenderGraph.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphCompiler.h"
#include "Common/RHISemaphore.h"

RenderGraph::RenderGraph()
{
}

TextureHandle RenderGraph::ImportTexture(const std::string &name, RHITexture *texture)
{
    TextureResource resource = {};
    resource.name = name;
    resource.texture = texture;
    resource.type = ResourceType::External;

    uint32_t index = static_cast<uint32_t>(textures.size());
    textures.push_back(std::move(resource));
    return { index };
}

BufferHandle RenderGraph::ImportBuffer(const std::string &name, RHIBuffer *buffer)
{
    BufferResource resource = {};
    resource.name = name;
    resource.buffer = buffer;
    resource.type = ResourceType::External;

    uint32_t index = static_cast<uint32_t>(buffers.size());
    buffers.push_back(std::move(resource));
    return { index };
}

TextureHandle RenderGraph::ImportTexture(const std::string &name, RHITexture *texture,
    std::vector<RHISemaphore *> wait_semaphores, std::vector<RHISemaphore *> signal_semaphores)
{
    TextureResource resource = {};
    resource.name = name;
    resource.texture = texture;
    resource.type = ResourceType::External;
    resource.external_wait_semaphore = std::move(wait_semaphores);
    resource.external_signal_semaphore = std::move(signal_semaphores);

    uint32_t index = static_cast<uint32_t>(textures.size());
    textures.push_back(std::move(resource));
    return { index };
}

BufferHandle RenderGraph::ImportBuffer(const std::string &name, RHIBuffer *buffer,
    std::vector<RHISemaphore *> wait_semaphores, std::vector<RHISemaphore *> signal_semaphores)
{
    BufferResource resource = {};
    resource.name = name;
    resource.buffer = buffer;
    resource.type = ResourceType::External;
    resource.external_wait_semaphore = std::move(wait_semaphores);
    resource.external_signal_semaphore = std::move(signal_semaphores);

    uint32_t index = static_cast<uint32_t>(buffers.size());
    buffers.push_back(std::move(resource));
    return { index };
}

TextureHandle RenderGraph::CreateTexture(const std::string &name, RHITextureDesc &texture_desc)
{
	uint32_t index = static_cast<uint32_t>(textures.size());

    TextureResource& resource = textures.emplace_back();
    resource.name = name;
    resource.texture = GetRHI()->RHICreateTexture(texture_desc);
    resource.type = ResourceType::Internal;

    texture_desc.DebugName = name.c_str();

    return { index };
}

BufferHandle RenderGraph::CreateBuffer(const std::string &name, const RHIBufferDesc &buffer_desc)
{
    BufferResource& resource = buffers.emplace_back();
    resource.name = name;
    resource.buffer = GetRHI()->RHICreateBuffer(buffer_desc);;
    resource.type = ResourceType::Internal;

    uint32_t index = static_cast<uint32_t>(buffers.size());
    return { index };
}

TextureHandle RenderGraph::GetTextureHandle(const std::string_view &name) const
{
    for (uint32_t index = 0; index < textures.size(); ++index)
    {
        if (textures[index].name == name)
        {
            return { index };
        }
    }
    return { 0xFFFFFFFF };
}

BufferHandle RenderGraph::GetBufferHandle(const std::string_view &name) const
{
    for (uint32_t index = 0; index < buffers.size(); ++index)
    {
        if (buffers[index].name == name)
        {
            return { index };
        }
    }
    return { 0xFFFFFFFF };
}

RHITexture * RenderGraph::ResolveTexture(TextureHandle texture_handle) const
{
    ensure(texture_handle.IsValid() && texture_handle.index < textures.size());
    return textures[texture_handle.index].texture;
}

RHIBuffer * RenderGraph::ResolveBuffer(BufferHandle buffer_handle) const
{
    ensure(buffer_handle.IsValid() && buffer_handle.index < buffers.size());
    return buffers[buffer_handle.index].buffer;
}

void RenderGraph::AddGraphicsPass(const std::string name, PassSetupFunc setup_callback,
                                  PassExecuteFunc execute_callback)
{
    RenderGraphPass pass;
    pass.name = name;
    pass.execute_func = std::move(execute_callback);

    RenderGraphBuilder builder(pass);
    setup_callback(builder);

    pass_data.push_back(std::move(pass));
}

void RenderGraph::Compile()
{
    RenderGraphCompiler compiler(*this);
    compiler.Compile();

    RenderGraphTimeline timeline = compiler.GetTimeline();

    std::vector<RenderGraphPass> new_pass_data;
    new_pass_data.reserve(pass_data.size());

    for (size_t index : timeline.topology_order)
    {
        new_pass_data.push_back(std::move(pass_data[index]));
    }

    pass_data = std::move(new_pass_data);
    pass_data.shrink_to_fit();
}

void RenderGraph::Execute()
{
    if (pass_data.empty())
    {
        return;
    }

    for (const auto& pass : pass_data)
    {
        command_list.Enqueue<RHICommandDebugMarkTimelineBegin>(pass.name.c_str());

        for (RHISemaphore* wait_semaphore : pass.wait_semaphores)
        {
            command_list.Enqueue<RHICommandAddWaitSemaphore>(wait_semaphore, RHIPipelineStage::TopOfPipe);
        }

        for (const auto& texture_barrier : pass.texture_barriers)
        {
            TextureResource& resource = textures[texture_barrier.texture_handle.index];
            RHITextureAccessState current_state = resource.texture_access_state;
            RHITextureAccessState target_state = texture_barrier.target_access_state;

            if (current_state == target_state)
            {
                continue;
            }

            command_list.Enqueue<RHICommandPipelineBarrierTexture>(resource.texture, current_state, target_state);

            resource.texture_access_state = target_state;
        }

        for (const auto& buffer_barrier : pass.buffer_barriers)
        {
            BufferResource& resource = buffers[buffer_barrier.buffer_handle.index];
            RHIBufferAccessState current_state = resource.buffer_access_state;
            RHIBufferAccessState target_state = buffer_barrier.target_access_state;

            if (current_state == target_state)
            {
                continue;
            }

            command_list.Enqueue<RHICommandPipelineBarrierBuffer>(resource.buffer, current_state, target_state);

            resource.buffer_access_state = target_state;
        }

        pass.execute_func(command_list);

        for (RHISemaphore* signal_semaphore : pass.signal_semaphores)
        {
            command_list.Enqueue<RHICommandAddSignalSemaphore>(signal_semaphore, RHIPipelineStage::AllCommands);
        }

        command_list.Enqueue<RHICommandDebugMarkTimelineEnd>();
    }
}

void RenderGraph::Reset()
{
    for (const auto& texture_resource : textures)
    {
        if (texture_resource.type == ResourceType::Internal)
        {
            delete texture_resource.texture;
        }
    }

	for (const auto& buffer_resource : buffers)
	{
		if (buffer_resource.type == ResourceType::Internal)
		{
			delete buffer_resource.buffer;
		}
	}

	pass_data.clear();
	textures.clear();
	buffers.clear();
}

RHICommandList & RenderGraph::GetCommandList()
{
    return command_list;
}