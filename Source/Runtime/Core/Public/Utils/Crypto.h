#pragma once

#include <cstdint>

struct SplitMix64
{
	uint64_t M_State;

	constexpr explicit SplitMix64(uint64_t P_Seed = 0)
		: M_State(P_Seed)
	{
	}

	uint64_t Next()
	{
		M_State += 0x9E3779B97F4A7C15ULL;
		uint64_t Z = M_State;
		Z = (Z ^ (Z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		Z = (Z ^ (Z >> 27)) * 0x94D049BB133111EBULL;
		return Z ^ (Z >> 31);
	}
};

struct Xoshiro128Plus
{
	uint32_t M_State[4];

	explicit Xoshiro128Plus(uint64_t P_Seed = 0)
	{
		Seed(P_Seed);
	}

	void Seed(uint64_t P_Seed)
	{
		SplitMix64 Seeder(P_Seed);
		uint64_t S0 = Seeder.Next();
		uint64_t S1 = Seeder.Next();
		M_State[0] = static_cast<uint32_t>(S0);
		M_State[1] = static_cast<uint32_t>(S0 >> 32);
		M_State[2] = static_cast<uint32_t>(S1);
		M_State[3] = static_cast<uint32_t>(S1 >> 32);
	}

	uint32_t Next()
	{
		uint32_t Result = M_State[0] + M_State[3];
		uint32_t T = M_State[1] << 9;

		M_State[2] ^= M_State[0];
		M_State[3] ^= M_State[1];
		M_State[1] ^= M_State[2];
		M_State[0] ^= M_State[3];

		M_State[2] ^= T;
		M_State[3] = (M_State[3] << 11) | (M_State[3] >> 21);

		return Result;
	}
};

struct PCG32
{
	uint64_t M_State;
	uint64_t M_Increment;

	explicit PCG32(uint64_t P_Seed = 0, uint64_t P_Sequence = 0)
		: M_State(0)
		, M_Increment((P_Sequence << 1) | 1ULL)
	{
		Next();
		M_State += P_Seed;
		Next();
	}

	uint32_t Next()
	{
		uint64_t Old = M_State;
		M_State = Old * 6364136223846793005ULL + M_Increment;
		uint32_t Xor_Shifted = static_cast<uint32_t>(((Old >> 18) ^ Old) >> 27);
		uint32_t Rot = static_cast<uint32_t>(Old >> 59);
		return (Xor_Shifted >> Rot) | (Xor_Shifted << ((-Rot) & 31));
	}
};