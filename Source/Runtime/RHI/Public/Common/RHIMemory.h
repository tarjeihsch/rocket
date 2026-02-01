#pragma once

using NativeHandle = uint64_t;

struct RHIBufferAllocation
{
	NativeHandle Handle = 0;

	AllocationInfo AllocInfo;

	uint64_t VirtualAddress;
};

struct RHITextureAllocation
{
	NativeHandle Handle = 0;
};

class RHIDeviceMemory
{
};