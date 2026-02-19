#pragma once

#include <assert.h>
#include <cmath>
#include <cstdint>
#include <time.h>

#ifdef _WIN32
	#include <Windows.h>
#endif

static constexpr uint64_t TICK_NS	= 1ULL;
static constexpr uint64_t TICK_US	= 1000ULL * TICK_NS;
static constexpr uint64_t TICK_MS	= 1000ULL * TICK_US;
static constexpr uint64_t TICK_S	= 1000ULL * TICK_MS;

static constexpr uint64_t TICK_M	= 60ULL * TICK_S;
static constexpr uint64_t TICK_H	= 60ULL * TICK_M;
static constexpr uint64_t TICK_D	= 24ULL * TICK_H;

struct Timespan
{
	uint64_t M_Ticks = 0;

	constexpr Timespan() = default;
	constexpr explicit Timespan(const uint64_t P_Ticks)
		: M_Ticks(P_Ticks)
	{
	}

	static constexpr Timespan FromNanoseconds(uint64_t P_Value) { return Timespan(P_Value * TICK_NS); }
	static constexpr Timespan FromMicroseconds(uint64_t P_Value) { return Timespan(P_Value * TICK_US); }
	static constexpr Timespan FromMilliseconds(uint64_t P_Value) { return Timespan(P_Value * TICK_MS); }
	static constexpr Timespan FromSeconds(uint64_t P_Value) { return Timespan(P_Value * TICK_S); }
	static constexpr Timespan FromMinutes(uint64_t P_Value) { return Timespan(P_Value * TICK_M); }
	static constexpr Timespan FromHours(uint64_t P_Value) { return Timespan(P_Value * TICK_H); }
	static constexpr Timespan FromDays(uint64_t P_Value) { return Timespan(P_Value * TICK_D); }

	constexpr double ToNanoseconds() const { return static_cast<double>(M_Ticks) / TICK_NS; }
	constexpr double ToMicroseconds() const { return static_cast<double>(M_Ticks) / TICK_US; }
	constexpr double ToMilliseconds() const { return static_cast<double>(M_Ticks) / TICK_MS; }
	constexpr double ToSeconds() const { return static_cast<double>(M_Ticks) / TICK_S; }
	constexpr double ToMinutes() const { return static_cast<double>(M_Ticks) / TICK_M; }
	constexpr double ToHours() const { return static_cast<double>(M_Ticks) / TICK_H; }
	constexpr double ToDays() const { return static_cast<double>(M_Ticks) / TICK_D; }

	constexpr Timespan operator+(Timespan P_Other) const { return Timespan(M_Ticks + P_Other.M_Ticks); }
	constexpr Timespan operator-(Timespan P_Other) const { return Timespan(M_Ticks - P_Other.M_Ticks); }
	constexpr bool operator<(Timespan P_Other)  const { return M_Ticks < P_Other.M_Ticks; }
	constexpr bool operator>(Timespan P_Other)  const { return M_Ticks > P_Other.M_Ticks; }
	constexpr bool operator<=(Timespan P_Other) const { return M_Ticks <= P_Other.M_Ticks; }
	constexpr bool operator>=(Timespan P_Other) const { return M_Ticks >= P_Other.M_Ticks; }
	constexpr bool operator==(Timespan P_Other) const { return M_Ticks == P_Other.M_Ticks; }
};

struct TimeDate
{
	uint64_t M_Ticks = 0;

	constexpr TimeDate() = default;
	constexpr explicit TimeDate(uint64_t P_Ticks) 
		: M_Ticks(P_Ticks) 
	{
	}

	constexpr Timespan operator-(TimeDate P_Other) const
	{
		return Timespan(M_Ticks - P_Other.M_Ticks);
	}

	constexpr TimeDate operator+(Timespan P_Timespan) const { return TimeDate(M_Ticks + P_Timespan.M_Ticks); }
	constexpr TimeDate operator-(Timespan P_Timespan) const { return TimeDate(M_Ticks - P_Timespan.M_Ticks); }

	constexpr bool operator<(TimeDate P_Other)  const { return M_Ticks < P_Other.M_Ticks; }
	constexpr bool operator>(TimeDate P_Other)  const { return M_Ticks > P_Other.M_Ticks; }
	constexpr bool operator<=(TimeDate P_Other) const { return M_Ticks <= P_Other.M_Ticks; }
	constexpr bool operator>=(TimeDate P_Other) const { return M_Ticks >= P_Other.M_Ticks; }
	constexpr bool operator==(TimeDate P_Other) const { return M_Ticks == P_Other.M_Ticks; }

	static TimeDate Now()
	{
		timespec Time_Now;
		timespec_get(&Time_Now, TIME_UTC);
		return TimeDate(static_cast<uint64_t>(Time_Now.tv_sec) * TICK_S + static_cast<uint64_t>(Time_Now.tv_nsec));
	}

	static TimeDate FromDate(int P_Year, int P_Month, int P_Day, int P_Hour = 0, int P_Minute = 0, int P_Second = 0);

	void ToDate(int &P_Year, int &P_Month, int &P_Day, int &P_Hour, int &P_Minute, int &P_Second) const;
};

struct Stopwatch
{
	static uint64_t GetTicks();
	static uint64_t GetFrequency();

	static Timespan GetDelta(uint64_t P_Start, uint64_t P_End)
	{
		uint64_t Ticks_Elapsed = P_End - P_Start;
		return Timespan((Ticks_Elapsed * TICK_S) / GetFrequency());
	}
};

inline TimeDate TimeDate::FromDate(int P_Year, int P_Month, int P_Day, int P_Hour, int P_Minute, int P_Second)
{
#ifdef _WIN32
	static constexpr uint64_t EPOCH_OFFSET = 116444736000000000ULL;

	SYSTEMTIME System_Time = {};
	System_Time.wYear = static_cast<WORD>(P_Year);
	System_Time.wMonth  = static_cast<WORD>(P_Month);
	System_Time.wDay = static_cast<WORD>(P_Day);
	System_Time.wHour = static_cast<WORD>(P_Hour);
	System_Time.wMinute = static_cast<WORD>(P_Minute);
	System_Time.wSecond = static_cast<WORD>(P_Second);

	FILETIME File_Time;
	if (!SystemTimeToFileTime(&System_Time, &File_Time))
	{
		return TimeDate(0);
	}

	uint64_t Ticks = (static_cast<uint64_t>(File_Time.dwHighDateTime) << 32) | File_Time.dwLowDateTime;

	if (Ticks < EPOCH_OFFSET)
	{
		// Underflow
		return TimeDate(0);
	}

	return TimeDate((Ticks - EPOCH_OFFSET) * 100ULL);
#endif

#ifdef __linux__
	tm Time_Epoch = {};
	Time_Epoch.tm_year = P_Year - 1900;
	Time_Epoch.tm_mon  = P_Month - 1;
	Time_Epoch.tm_mday = P_Day;
	Time_Epoch.tm_hour = P_Hour;
	Time_Epoch.tm_min  = P_Minute;
	Time_Epoch.tm_sec  = P_Second;

	time_t Time_Since_Epoch = timegm(&Time_Epoch);

	if (Time_Since_Epoch == (time_t)(-1))
	{
		return TimeDate(0);
	}

	if (Time_Since_Epoch < 0)
	{
		return TimeDate(0);
	}

	return TimeDate(static_cast<uint64_t>(Time_Since_Epoch) * TICK_S);
#endif
}

inline void TimeDate::ToDate(int& P_Year, int& P_Month, int& P_Day, int& P_Hour, int& P_Minute, int& P_Second) const
{
#ifdef _WIN32
	static constexpr uint64_t EPOCH_OFFSET = 116444736000000000ULL;
	uint64_t Ticks_100ns = (M_Ticks / 100ULL) + EPOCH_OFFSET;

	FILETIME File_Time;
	File_Time.dwLowDateTime  = static_cast<DWORD>(Ticks_100ns);
	File_Time.dwHighDateTime = static_cast<DWORD>(Ticks_100ns >> 32);

	SYSTEMTIME System_Time;
	if (!FileTimeToSystemTime(&File_Time, &System_Time))
	{
		P_Year = P_Month = P_Day = P_Hour = P_Minute = P_Second = 0;
		return;
	}

	P_Year   = System_Time.wYear;
	P_Month  = System_Time.wMonth;
	P_Day    = System_Time.wDay;
	P_Hour   = System_Time.wHour;
	P_Minute = System_Time.wMinute;
	P_Second = System_Time.wSecond;
#endif

#ifdef __linux__
	time_t Epoch = static_cast<time_t>(M_Ticks / TICKS_PER_SECOND);
	tm Time_Result = {};

	if (!gmtime_r(&Epoch, &Time_Result))
	{
		P_Year = P_Month = P_Day = P_Hour = P_Minute = P_Second = 0;
		return;
	}

	P_Year   = Time_Result.tm_year + 1900;
	P_Month  = Time_Result.tm_mon + 1;
	P_Day    = Time_Result.tm_mday;
	P_Hour   = Time_Result.tm_hour;
	P_Minute = Time_Result.tm_min;
	P_Second = Time_Result.tm_sec;

#endif
}

#ifdef _WIN32

inline uint64_t Stopwatch::GetTicks()
{
	LARGE_INTEGER Ticks;
	QueryPerformanceCounter(&Ticks);
	return static_cast<uint64_t>(Ticks.QuadPart);
}

inline uint64_t Stopwatch::GetFrequency()
{
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);
	return static_cast<uint64_t>(Frequency.QuadPart);
}

#endif

#ifdef __linux__

#include <time.h>

inline uint64_t Stopwatch::GetTicks()
{
	timespec Ticks;
	clock_gettime(CLOCK_MONOTONIC, &Ticks);
	return static_cast<uint64_t>(Ticks.tv_sec) * TICKS_PER_SECOND + Ticks.tv_nsec;
}

inline uint64_t Stopwatch::GetFrequency()
{
	return TICKS_PER_SECOND;
}

#endif