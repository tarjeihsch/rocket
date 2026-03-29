#pragma once

#include <cstdint>

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
		data |= ((uint32_t)r << 24);
		data |= ((uint32_t)g << 16);
		data |= ((uint32_t)b << 8);
		data |= ((uint32_t)a << 0);
	}

	static const Color32 red;
	static const Color32 green;
	static const Color32 blue;
};

inline const Color32 Color32::red(255, 0, 0, 255);
inline const Color32 Color32::green(0, 255, 0, 255);
inline const Color32 Color32::blue(0, 0, 255, 255);