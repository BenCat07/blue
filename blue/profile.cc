#include <stdafx.hh>

#include "profile.hh"

#include "convar.hh"

using namespace Profiler;

u64   Timer::clock_speed;
float Timer::clock_multiplier_micro;
float Timer::clock_multiplier_milli;
float Timer::clock_multiplier_whole;

auto Timer::calculate_clock_speed() -> void {
    LARGE_INTEGER waitTime, startCount, curCount;

    Timer t;

    // Take 1/32 of a second for the measurement.
    QueryPerformanceFrequency(&waitTime);
    int scale = 5;
    waitTime.QuadPart >>= scale;

    QueryPerformanceCounter(&startCount);
    t.start();
    {
        do {
            QueryPerformanceCounter(&curCount);
        } while (curCount.QuadPart - startCount.QuadPart < waitTime.QuadPart);
    }
    t.end();

    clock_speed = t.cycles() << scale;

    // Deal with the multipliers here aswell...
    clock_multiplier_whole = 1.0f / clock_speed;
    clock_multiplier_milli = 1000.0f / clock_speed;
    clock_multiplier_micro = 1000000.0f / clock_speed;
}

// Calculate the clockspeed on init
init_time(Timer::calculate_clock_speed());

static auto profiling_enabled = Convar<bool>("blue_profiling_enabled", true, nullptr);
auto        ProfileScope::profiling_enabled() -> bool {
    return ::profiling_enabled == true;
}
