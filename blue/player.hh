#pragma once

#include "types.hh"

#include "entity.hh"

#undef min
#undef max

namespace TF {
struct PlayerHitboxes {
    Math::Vector centre[128];

    Math::Vector min[128];
    Math::Vector max[128];

    // TODO: temporary fields
    Math::Vector origin[128];
    Math::Vector rotation[128];
};

class Player : public Entity {
public:
    Player() = delete;

    // helper functions
    static auto local() -> Player *;

    auto world_space_centre() -> Math::Vector &;

    auto model_handle() -> const class ModelHandle *;
    auto studio_model() -> const class StudioModel *;

    auto view_position() -> Math::Vector;

    auto hitboxes(PlayerHitboxes *hitboxes_out, bool create_pose) -> u32;

    auto can_shoot() -> bool;

    // netvars
    auto health() -> int &;

    auto alive() -> bool;
    auto team() -> int;

    auto view_offset() -> Math::Vector &;

    auto tf_class() -> int;

    auto tick_base() -> int;
    auto sim_time() -> float &;
    auto anim_time() -> float &;
    auto cycle() -> float &;
    auto sequence() -> int &;

    auto next_attack_after_reload() -> float;

    // TODO: return Weapon *
    auto active_weapon() -> Entity *;

    // virtual functions
    auto origin() -> Math::Vector &;
    auto set_origin(const Math::Vector &v) -> void;

    auto render_bounds() -> std::pair<Math::Vector, Math::Vector>;

    auto angles() -> Math::Vector &;
    auto set_angles(const Math::Vector &v) -> void;

    auto anim_layer(u32 index) -> class AnimationLayer &;
    auto anim_layer_count() -> u32;

    // These are relative to the origin
    auto collision_bounds() -> std::pair<Math::Vector &, Math::Vector &>;

    auto bone_transforms(Math::Matrix3x4 *hitboxes_out, u32 max_bones, u32 bone_mask, float current_time) -> bool;
};
} // namespace TF
