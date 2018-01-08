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
    static auto local() -> Player *;

    auto world_space_centre() -> Math::Vector &;

    auto model_handle() -> const class ModelHandle *;
    auto studio_model() -> const class StudioModel *;

    auto view_position() -> Math::Vector;

    struct PlayerHitboxes {
        Math::Vector centre[128];

        Math::Vector min[128];
        Math::Vector max[128];

        // TODO: temporary fields
        Math::Vector origin[128];
        Math::Vector rotation[128];
    };

    auto hitboxes(PlayerHitboxes *hitboxes_out, bool create_pose) -> u32;

    // netvars
    auto health() -> int &;

    auto alive() -> bool;
    auto team() -> int;

    auto view_offset() -> Math::Vector &;

    // TODO: return Weapon *
    auto active_weapon() -> Entity *;

    // virtual functions
    auto origin() -> Math::Vector &;
    auto render_bounds() -> std::pair<Math::Vector, Math::Vector>;

    auto bone_transforms(Math::Matrix3x4 *hitboxes_out, u32 max_bones, u32 bone_mask, float current_time) -> bool;
};
} // namespace TF
