#pragma once

#include <cstdint>

enum class ERHIQueueSyncStage : uint8_t
{
    TopOfPipe,
    ColorAttachmentOutput,
    AllCommands
};

struct RHISemaphore
{
    virtual ~RHISemaphore() = default;
    
    virtual void* GetHandle() = 0;
};