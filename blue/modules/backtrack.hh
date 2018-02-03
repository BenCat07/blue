#pragma once

#include "blue_platform.hh"

// Backtrack stuff

// TODO:
// Get visible information from target manager so we only calculate that once
// Some sort of iterable range for seeing whether a target is visible

// Interface to allow rollbacks of entity data

namespace TF {
class UserCmd;
class Player;
struct PlayerHitboxes;
} // namespace TF

class Backtrack {
public:
    static auto create_move(TF::UserCmd *cmd) -> void;

    static auto level_startup() -> void;
    static auto level_shutdown() -> void;

    static auto update_all() -> void;

    static auto update_player_hitboxes(TF::PlayerHitboxes *h) -> void;

    static auto backtrack_player_to_tick(TF::Player *p, u32 tick) -> bool;
};
