#pragma once
// platform helpers

// we define both macros and constexpr functions
// you should always prefer to use an "if constexpr()"
// as opposed to a "#if"

// HOWEVER: there are situations where a macro check makes more sense
// e.g.

// clang-format off
#define blueplatform_clang() false
#define blueplatform_msvc() false
#define blueplatform_gcc() false

#if defined(__clang__)
	#undef blueplatform_clang
	#define blueplatform_clang() true
#elif defined(_MSC_VER)
	#undef blueplatform_msvc
	#define blueplatform_msvc() true
#elif defined(__GNUC__)
	#undef blueplatform_gcc
	#define blueplatform_gcc() true
#else
    #error Unknown compiler: fix me!
#endif

#define blueplatform_windows() false
#define blueplatform_linux() false
#define blueplatform_osx() false

#if defined(_WIN32)
	#undef blueplatform_windows
	#define blueplatform_windows() true
#elif defined(__unix__) && !defined(__APPLE__)
	#undef blueplatform_linux
	#define blueplatform_linux() true
#elif defined(__APPLE__)
	#undef blueplatform_osx
	#define blueplatform_osx() true
#else
    #error Unknown platform: fix me!
#endif
// clang-format on

namespace BluePlatform {
constexpr bool windows() {
    return blueplatform_windows();
}

constexpr bool linux() {
    return blueplatform_linux();
}

constexpr bool osx() {
    return blueplatform_osx();
}

constexpr bool msvc() {
    return blueplatform_msvc();
}

constexpr bool clang() {
    return blueplatform_clang();
}

constexpr bool gcc() {
    return blueplatform_gcc();
}
} // namespace BluePlatform

// ALWAYS use the types from here unless you are dealing with
// an interface that forces you to ues other types
#include "types.h"

#if blueplatform_windows()
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif