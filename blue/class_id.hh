#pragma once

#include "blue_platform.hh"

// This file defines helpers and functionality for using and updating classids

namespace TF {
namespace ClassID {

// See comment blow for #define ID
namespace InternalChecker {
class ClassIDChecker {
    static ClassIDChecker *head;

public:
    const char *    name;
    u32             intended_value;
    ClassIDChecker *next = nullptr;

    ClassIDChecker(const char *name, const u32 value);

    auto        check_correct() -> bool;
    static auto check_all_correct() -> void;
};
} // namespace InternalChecker

// For debug builds we want to be able to check our classids are correct and issue warnings if they are not correct
// So that we can update the value for next time.
#if defined(_DEBUG) && defined(PLACE_CHECKER)
#define ID(name, value)                          \
    enum { name = value };                       \
    namespace InternalChecker {                  \
    inline auto checker_##name = ClassIDChecker( \
        #name,                                   \
        value);                                  \
    }
#else
#define ID(name, value) \
    enum { name = value };
#endif

// Put ids here
ID(CTFPlayer, 246);
ID(CTFRevolver, 284);
ID(CTFSniperRifle, 305);

#undef ID

} // namespace ClassID
} // namespace TF
