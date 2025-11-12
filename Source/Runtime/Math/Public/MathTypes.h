#pragma once

static inline float Floor(float Value)
{
	return (float)(((Value >= 0 || (float)(int)Value == Value) ? (int)Value : (int)Value - 1));
}