#pragma once

#include "types.h"

namespace Signature {
auto resolve_library(const char *name) -> void *;
auto resolve_import(void *handle, const char *name) -> void *;

auto find_pattern(const char *module, const char *pattern) -> u8 *;
auto find_pattern(const char *pattern, uptr start, uptr length) -> u8 *;

template <typename T>
auto find_pattern(const char *module, const char *pattern, u32 offset) -> T {
    return reinterpret_cast<T>(find_pattern(module, pattern) + offset);
}
auto resolve_callgate(void *address) -> void *;

class Pattern {
    const char *module;
    const char *signature;

public:
    Pattern(const char *module,
            const char *signature_windows,
            const char *signature_linux,
            const char *signature_osx)
        : module(module),
#if blueplatform_windows()
          signature(signature_windows)
#elif blueplatform_linux()
          signature(signature_linux)
#elif blueplatform_osx()
          signature(signature_osx)
#endif
    {
        assert(signature[0] != '\0');
    }

    auto find() -> u8 * {
        return ::Signature::find_pattern(module, signature);
    }

    template <typename T>
    auto find(u32 offset) -> T {
        return ::Signature::find_pattern<T>(module, signature, offset);
    }
};
} // namespace Signature