#pragma once

#include "vfunc.h"

namespace TF {
class Entity {
public:
    Entity() = delete;

    // helper functions
    auto is_valid() -> bool;

    // netvars
    auto health() -> int &;
};
} // namespace TF