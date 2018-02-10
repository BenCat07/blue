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
    // The max number of ticks you can go back
    enum { max_ticks = 15 };

    //static auto create_move(TF::UserCmd *cmd) -> void;

    static auto level_startup() -> void;
    static auto level_shutdown() -> void;

    static auto create_move_pre_predict(TF::UserCmd *cmd) -> void;
    static auto create_move(TF::UserCmd *cmd) -> void;
    static auto create_move_finish(TF::UserCmd *cmd) -> void;

    static auto update_player_hitboxes(TF::Player *p, TF::PlayerHitboxes *h, u32 max_hitboxes) -> void;

    static auto backtrack_player_to_tick(TF::Player *p, u32 tick, bool restoring = false) -> bool;
    static auto get_hitboxes_for_player_at_tick(TF::Player *p, u32 tick, TF::PlayerHitboxes *h) -> void;
};
