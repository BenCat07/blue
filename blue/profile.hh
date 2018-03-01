#pragma once

#include "blue_platform.hh"

#include "log.hh"

// TODO: do we even want this visual studio does profiling perfectly well...

// Simple profiling class
namespace Profiler {
// Simple timer
class Timer {
    static u64   clock_speed;
    static float clock_multiplier_micro;
    static float clock_multiplier_milli;
    static float clock_multiplier_whole;

    u64 begin_count;
    u64 duration;

public:
    // Init for calculating clock speed
    static auto calculate_clock_speed() -> void;

    auto sample() { return __rdtsc(); }

    auto start() {
        begin_count = sample();
    }
    auto end() {
        auto end_count = sample();
        duration       = end_count - begin_count;
    }

    auto reset() {
        begin_count = 0;
    }

    auto cycles() { return duration; }
    auto milliseconds() { return (duration * 1000) / clock_speed; }
};

// TODO: maybe we want a node / network model like in vprof
class ProfileScope {
#ifdef _DEBUG
    Timer       t;
    const char *name;

public:
    ProfileScope(const char *name) : name(name) { t.start(); }

    ~ProfileScope() {
        t.end();

        auto cycles       = t.cycles();
        auto milliseconds = t.milliseconds();

        if (profiling_enabled()) Log::msg("[Profiler] [%s] %llu %f", name, cycles, milliseconds);
    }

#endif
    auto profiling_enabled() -> bool;
};

#define PROFILE_FUNCTION() auto scope_##__LINE__ = Profiler::ProfileScope(__FUNCTION__)

} // namespace Profiler
