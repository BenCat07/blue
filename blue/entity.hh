#pragma once

#include "types.hh"
#include "vfunc.hh"

namespace TF {
struct EntityHandle {
    u32 serial_index;
};

class Entity {
public:
    Entity() = delete;

    // helper functions
    auto is_valid() -> bool;

    auto to_handle() -> EntityHandle &;

    template <typename T, u32 offset>
    auto set(T data) { *reinterpret_cast<T *>(reinterpret_cast<uptr>(this) + offset) = data; }

    template <typename T, u32 offset>
    auto get() { return *reinterpret_cast<T *>(reinterpret_cast<uptr>(this) + offset); }

    // upcasts
    auto to_player() -> class Player *;

    // virtual functions
    auto client_class() -> struct ClientClass *;

    auto dormant() -> bool;
    auto index() -> u32;
};
} // namespace TF
