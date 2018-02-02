#pragma once

#include "sdk.hh"

#include "blue_target_list.hh"

class Aimbot {
public:
    static auto init_all() -> void;

    static auto update(float frametime) -> void;

    static auto create_move(TF::UserCmd *cmd) -> void;
    static auto create_move_pre_predict(TF::UserCmd *cmd) -> void;

    static auto flush_targets() -> void;
    static auto valid_target(TF::Entity *e, bool &is_valid) -> void;
    static auto visible_target(TF::Entity *e, Math::Vector &position, bool &is_visible) -> void;
    static auto finished_target(BlueTarget::Target t) -> void;
    static auto sort_targets() -> void;
};
