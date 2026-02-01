#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

#include "RHIDescriptorResource.h"

class RHIDescriptorAllocator
{
public:
	RHIDescriptorAllocator(uint32_t InCapacity)
		: Capacity(InCapacity)
	{
	}

	void Init()
	{
		Next.resize(Capacity, 0xFFFFFFFFu);
		Version.resize(Capacity, 1);

		Head = 0xFFFFFFFFu;

		for (uint32_t Index = Capacity; Index-- > 0u; )
		{
			Next[Index] = Head;
			Head = Index;
		}
	}

	RHIDescriptorHandle Allocate()
	{
		if (Head == 0xFFFFFFFFu)
		{
			return {};
		}

		const uint32_t Index = Head;
		Head = Next[Index];

		RHIDescriptorHandle Handle;
		Handle.Index = Index;
		Handle.Version = Version[Index];

		return Handle;
	}

	bool Free(const RHIDescriptorHandle& Handle)
	{
		if (Handle.Index >= Capacity)
		{
			return false;
		}

		Version[Handle.Index] += 1;

		Next[Handle.Index] = Head;
		Head = Handle.Index;
	}

	bool Validate(const RHIDescriptorHandle& Handle) const
	{
		if (Handle.Index >= Capacity)
		{	
			return false;
		}

		return Version[Handle.Index] == Handle.Version;
	}


	void Reset()
	{
		Next.clear();
		Version.clear();
		Head = 0xFFFFFFFFu;
	}


private:
	std::vector<uint32_t> Next;
	std::vector<uint32_t> Version;

	uint32_t Head;
	uint32_t Capacity;
};