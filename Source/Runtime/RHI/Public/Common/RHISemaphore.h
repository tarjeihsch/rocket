#pragma once

#include <cstdint>

enum class RHISyncStage : uint8_t
{
    TopOfPipe,
    ColorAttachmentOutput,
    AllCommands
};

struct RHISemaphore
{
    virtual ~RHISemaphore() = default;
    
    virtual void* GetNativeHandle() = 0;
};