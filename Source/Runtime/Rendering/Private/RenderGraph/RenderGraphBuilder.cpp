#include "RenderGraphBuilder.h"

#include "RenderGraphPass.h"

RenderGraphBuilder::RenderGraphBuilder(RenderGraphPass &in_pass)
    : pass(in_pass)
{
}

void RenderGraphBuilder::ReadTexture(TextureHandle handle)
{
    pass.texture_barriers.push_back({ handle, RHITextureAccessState::ShaderRead() });
    pass.texture_reads.push_back(handle);
}

void RenderGraphBuilder::WriteTexture(TextureHandle handle)
{
    pass.texture_barriers.push_back({ handle, RHITextureAccessState::UnorderedAccess() });
    pass.texture_writes.push_back(handle);
}

void RenderGraphBuilder::WriteColorAttachment(TextureHandle handle)
{
    pass.texture_barriers.push_back({ handle, RHITextureAccessState::ColorAttachment() });
    pass.texture_writes.push_back(handle);
}

void RenderGraphBuilder::WriteDepthAttachment(TextureHandle handle)
{
    pass.texture_barriers.push_back({ handle, RHITextureAccessState::DepthStencilAttachment() });
    pass.texture_writes.push_back(handle);
}

void RenderGraphBuilder::ReadBuffer(BufferHandle handle)
{
    pass.buffer_barriers.push_back({ handle, RHIBufferAccessState::ShaderRead() });
    pass.buffer_reads.push_back(handle);
}

void RenderGraphBuilder::WriteBuffer(BufferHandle handle)
{
    pass.buffer_barriers.push_back({ handle, RHIBufferAccessState::UnorderedAccess() });
    pass.buffer_writes.push_back(handle);
}

void RenderGraphBuilder::CopySrcTexture(TextureHandle handle)
{
	pass.texture_barriers.push_back({ handle, RHITextureAccessState::CopySrc() });
	pass.texture_reads.push_back(handle);
}

void RenderGraphBuilder::CopyDstTexture(TextureHandle handle)
{
	pass.texture_barriers.push_back({ handle, RHITextureAccessState::CopyDst() });
	pass.texture_writes.push_back(handle);
}

void RenderGraphBuilder::Present(TextureHandle handle)
{
	pass.texture_barriers.push_back({ handle, RHITextureAccessState::Present() });
	pass.texture_writes.push_back(handle);
}
