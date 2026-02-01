#pragma once

#include <cstdint>
#include <optional>

struct FRHISRVDesc;
struct FRHIUAVDesc;
struct FRHICBVDesc;
struct FRHISamplerDesc;

class RHIResource;
class RHITexture;

struct RHIDescriptorHandle
{
	uint32_t Index = 0xFFFFFFFFu;
	uint32_t Version = 0xFFFFFFFFu;

	inline bool IsValid() const { return Index != 0xFFFFFFFFu; }
};

enum RHIDescriptorType : uint32_t
{
	SRV = 0,

	UAV = 1,

	SSV = 2,

	Count
};

class RHIDescriptorResource
{
public:
	virtual ~RHIDescriptorResource() = default;

	const RHIDescriptorType GetDescriptorType() const noexcept { return Type; }
	const RHIDescriptorHandle& GetDescriptorHandle() const noexcept { return DescriptorHandle; }

	uint32_t GetContentVersion() const noexcept { return ContentVersion; }
	uint32_t GetResidentVersion() const noexcept { return ResidentVersion; }

	void MarkDirty() noexcept
	{
		ContentVersion += 1;
	}

	bool IsDirty() const noexcept
	{
		return GetResidentVersion() != GetContentVersion();
	}

	void SetResidentHandle(const RHIDescriptorHandle& InDescriptorHandle, uint32_t InResidentVersion) noexcept
	{
		DescriptorHandle = InDescriptorHandle;
		ResidentVersion = InResidentVersion;
	}

	void ClearResidentHandle() noexcept
	{
		DescriptorHandle = {};
		ResidentVersion = 0;
	}

protected:
	explicit RHIDescriptorResource(RHIDescriptorType InType) noexcept
		: Type(InType)
	{
	}

private:
	const RHIDescriptorType Type;

	uint32_t ContentVersion = 1;
	uint32_t ResidentVersion = 0;

	RHIDescriptorHandle DescriptorHandle;
};

class RHIResourceViewDescriptor : public RHIDescriptorResource
{
public:
	const RHIResource* GetResource() const noexcept { return Resource; }

protected:
	RHIResourceViewDescriptor(RHIDescriptorType InType, const RHIResource* InResource) noexcept
		: RHIDescriptorResource(InType)
		, Resource(InResource)
	{
	}

protected:
	const RHIResource* Resource;
};

struct FRHISRVDesc
{
	RHITexture* Texture;
	uint32_t MostDetailedMip;
	uint32_t MipLevels;
	uint32_t FirstArraySlice;
	uint32_t ArraySize;
	//EPixelFormat Format;
};

class RHIShaderResourceView final : public RHIResourceViewDescriptor
{
public:
	RHIShaderResourceView(const RHIResource* InResource, const FRHISRVDesc& InDesc)
		: RHIResourceViewDescriptor(RHIDescriptorType::SRV, InResource)
		, Desc(InDesc)
	{
	}

	void UpdateDesc(const FRHISRVDesc& InDesc)
	{
		Desc = InDesc;
		MarkDirty();
	}

private:
	FRHISRVDesc Desc;
};