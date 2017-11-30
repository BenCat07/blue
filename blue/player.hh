#pragma once

#include "types.hh"

#include "entity.hh"

#undef min
#undef max

namespace TF {
class Player : public Entity {
public:
    Player() = delete;

    // helper functions
    auto world_space_centre() -> Math::Vector &;

    static auto local() -> Player *;

    // netvars
    auto health() -> int &;

    auto alive() -> bool;
    auto team() -> int;

    // virtual functions
    auto origin() -> Math::Vector &;
    auto render_bounds() -> std::pair<Math::Vector, Math::Vector>;
};
} // namespace TF
