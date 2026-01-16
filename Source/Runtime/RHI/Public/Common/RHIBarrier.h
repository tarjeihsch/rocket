#pragma once

struct RHIBufferAccessState;
struct RHITextureAccessState;
class RHICommandList;
struct RHIBuffer;
struct RHITexture;

class RHIBarrier
{
public:
	virtual ~RHIBarrier() = default;

	virtual void AddTextureTransition(RHITexture* texture, const RHITextureAccessState &old_access_state, const RHITextureAccessState &new_access_state) = 0;
	virtual void AddBufferTransition(RHIBuffer* buffer, const RHIBufferAccessState &old_access_state, const RHIBufferAccessState &new_access_state) = 0;
	virtual void Execute(RHICommandList &cmd_list) = 0;
};
