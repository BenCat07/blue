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

#ifdef _DEBUG
    // These are used for debugging
    Math::Vector origin[128];
    Math::Vector rotation[128];

    Math::Vector raw_min[128];
    Math::Vector raw_max[128];
#endif
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

    // netvars
    auto health() -> int &;

    auto alive() -> bool;
    auto team() -> int;

    auto cond() -> u32 &;

    auto view_offset() -> Math::Vector &;

    auto tf_class() -> int;

    auto tick_base() -> int;
    auto sim_time() -> float &;
    auto anim_time() -> float &;
    auto cycle() -> float &;
    auto sequence() -> int &;

    auto fov_time() -> float;

    auto render_origin() -> Math::Vector &;

    auto active_weapon() -> Weapon *;

    // virtual functions
    auto origin() -> Math::Vector &;
    auto set_origin(const Math::Vector &v) -> void;

    auto render_bounds() -> std::pair<Math::Vector, Math::Vector>;

    auto angles() -> Math::Vector &;
    auto set_angles(const Math::Vector &v) -> void;

    auto anim_layer(u32 index) -> class AnimationLayer &;
    auto anim_layer_count() -> u32;

    auto update_client_side_animation() -> void;

    auto invalidate_physics_recursive(u32 flags) -> void;

    // These are relative to the origin
    auto collision_bounds() -> std::pair<Math::Vector &, Math::Vector &>;

    auto bone_transforms(Math::Matrix3x4 *hitboxes_out, u32 max_bones, u32 bone_mask, float current_time) -> bool;
};
} // namespace TF
