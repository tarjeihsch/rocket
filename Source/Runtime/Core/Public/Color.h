#pragma once

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