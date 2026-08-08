#pragma once

#include <cstdint>
#include "Math/Vector.h"

static float Clamp01(float value)
{
	return Math::Clamp(value, 0.0f, 1.0f);
}

static uint8_t ToByte(float value)
{
	return (uint8_t)(Clamp01(value) * 255.0f + 0.5f);
}

struct LinearColor
{
	float R;
	float G;
	float B;
	float A;

	LinearColor()
		: R(0), G(0), B(0), A(0)
	{
	}

	LinearColor(float InR, float InG, float InB, float InA)
		: R(InR), G(InG), B(InB), A(InA)
	{
	}

	static const LinearColor Black;
	static const LinearColor White;
	static const LinearColor Red;
	static const LinearColor Green;
	static const LinearColor Blue;
};

inline const LinearColor LinearColor::Black(0, 0, 0, 1);
inline const LinearColor LinearColor::White(1, 1, 1, 1);
inline const LinearColor LinearColor::Red(1, 0, 0, 1);
inline const LinearColor LinearColor::Green(0, 1, 0, 1);
inline const LinearColor LinearColor::Blue(0, 0, 1, 1);

struct Color32
{
	uint32_t data;

	Color32()
	{
		data = 0;
	}

	Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		data = 0;
		data |= ((uint32_t)a << 24);
		data |= ((uint32_t)b << 16);
		data |= ((uint32_t)g << 8);
		data |= ((uint32_t)r << 0);
	}

	// Converts a linear-space color to Color32, gamma-encoding RGB to sRGB.
	// Alpha is stored linearly (never gamma-encoded).
	explicit Color32(const LinearColor& c)
		: Color32(ToByte(c.R), ToByte(c.G), ToByte(c.B), ToByte(c.A))
	{
	}

	static const Color32 red;
	static const Color32 green;
	static const Color32 blue;
};

inline const Color32 Color32::red(255, 0, 0, 255);
inline const Color32 Color32::green(0, 255, 0, 255);
inline const Color32 Color32::blue(0, 0, 255, 255);