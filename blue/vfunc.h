#pragma once

#include <functional>
#include <type_traits>

#include "blue_platform.h"
#include "types.h"

// helpers for calling virtual functions
namespace VFunc {
inline auto get_table(void *inst, u32 offset) -> void **& {
    return *reinterpret_cast<void ***>((uptr)inst + offset);
}

inline auto get_table(const void *inst, u32 offset) -> const void **& {
    return *reinterpret_cast<const void ***>((uptr)inst + offset);
}

template <typename F>
inline auto get_func(const void *inst, u32 index, u32 offset) -> F {
    return reinterpret_cast<F>(get_table(inst, offset)[index]);
}

template <typename F>
inline auto get_func(void *inst, u32 index, u32 offset) -> F {
    return reinterpret_cast<F>(get_table(inst, offset)[index]);
}

template <typename>
class Func;

template <typename object_type, typename ret, typename... args>
class Func<ret (object_type::*)(args...)> {

    using function_type = ret(__thiscall *)(object_type *, args...);
    function_type f;

public:
    // TODO: do any other platforms have this offset and is it useful?
    Func(object_type *instance,
         u32          index_windows,
         u32          index_linux,
         u32          index_osx,
         u32          offset_windows) {
        assert(instance != nullptr);

        auto index = 0u;
        if constexpr (BluePlatform::windows())
            index = index_windows;
        else if constexpr (BluePlatform::linux())
            index = index_linux;
        else if constexpr (BluePlatform::osx())
            index = index_osx;

        auto offset = 0u;
        if constexpr (BluePlatform::windows())
            offset = offset_windows;

        f = get_func<function_type>(instance, index, offset);
    }

    auto invoke(object_type *instance, args... arguments) -> ret {
        return f(instance, arguments...);
    }
};

} // namespace VFunc

#define return_virtual_func(name, windows, linux, osx, off, ...) \
    using c = std::remove_reference<decltype(*this)>::type;      \
    using t = decltype(&c::name);                                \
    return VFunc::Func<t>(this, windows, linux, osx, off).invoke(this, __VA_ARGS__)