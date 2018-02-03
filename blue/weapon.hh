#pragma once

#include "entity.hh"

namespace TF {
class Weapon : public Entity {
public:
    auto next_primary_attack() -> float;
    auto next_secondary_attack() -> float;

    auto owner() -> Entity *;
};
} // namespace TF
