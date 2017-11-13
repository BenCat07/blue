#include "stdafx.h"

#include "gamesystem.h"
#include "log.h"

class Blue_GameSystem : public GameSystem {
public:
    auto init() -> bool override {
        Log::msg("init()");
        return true;
    }
    auto post_init() -> void override { Log::msg("post_init()"); }
    auto shutdown() -> void override { Log::msg("shutdown()"); }

    auto level_init_pre_entity() -> void override { Log::msg("init_pre_entity"); }
    auto level_init_post_entity() -> void override { Log::msg("level_init_post_entity"); }
    auto level_shutdown_pre_clear_steam_api_context() -> void override { Log::msg("level_shutdown_pre_clear_steam_api_context"); }
    auto level_shutdown_pre_entity() -> void override { Log::msg("level_shutdown_pre_entity"); }
    auto level_shutdown_post_entity() -> void override { Log::msg("level_shutdown_post_entity"); }

    auto update(float frametime) -> void override { Log::msg("%f", frametime); }

    Blue_GameSystem() {
        GameSystem::add_all();
    }
};

Blue_GameSystem blue_system;