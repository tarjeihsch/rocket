#pragma once

#include <string>

#include "Common/RHIBuffer.h"
#include "Common/RHITexture.h"

struct RHISemaphore;
class RenderGraphBuilder;
class RHICommandList;

enum class QueueType : uint8_t
{
	Graphics,
	Compute,
	Copy,
	Count
};

enum class ResourceType
{
	Internal,
	External
};

struct Resource
{
	std::string name;
	ResourceType type;

	std::vector<RHISemaphore*> external_wait_semaphore;
	std::vector<RHISemaphore*> external_signal_semaphore;
};

struct TextureResource : Resource
{
	RHITexture* texture;
	RHITextureAccessState texture_access_state;
};

struct BufferResource : Resource
{
	RHIBuffer* buffer;
	RHIBufferAccessState buffer_access_state;
};

template<typename T>
struct ResourceHandle
{
	uint32_t index = 0xFFFFFFFF;

	bool IsValid() const noexcept
	{
		return index != 0xFFFFFFFF;
	}
};

using TextureHandle = ResourceHandle<TextureResource>;
using BufferHandle = ResourceHandle<BufferResource>;

struct TextureBarrierInfo
{
	TextureHandle texture_handle;
	RHITextureAccessState target_access_state;
};

struct BufferBarrierInfo
{
	BufferHandle buffer_handle;
	RHIBufferAccessState target_access_state;
};

using PassSetupFunc = std::function<void(RenderGraphBuilder&)>;
using PassExecuteFunc = std::function<void(RHICommandList&)>;