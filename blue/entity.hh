#pragma once

#include "types.hh"
#include "vfunc.hh"

namespace TF {
class Entity {
public:
    Entity() = delete;

    // helper functions
    auto is_valid() -> bool;

    // upcasts
    auto to_player() -> class Player *;

    // virtual functions
    auto client_class() -> struct ClientClass *;
};
} // namespace TF
