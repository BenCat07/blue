#include "stdafx.h"

#include "gamesystem.h"
#include "log.h"

#include "interface.h"
#include "vfunc.h"

class EngineClient {
public:
    auto get_last_timestamp() -> float {
        return_virtual_func(get_last_timestamp, 15, 0, 0, 0);
    }
};

class Blue_GameSystem : public GameSystem {
public:
    auto init() -> bool override {
        Log::msg("init()");
        return true;
    }
    auto post_init() -> void override {
        Log::msg("post_init()");
        IFace<EngineClient>().set_interface("Engine", "VEngineClient");
        Log::msg("last_timestamp: %f", IFace<EngineClient>()->get_last_timestamp());
    }
    auto shutdown() -> void override { Log::msg("shutdown()"); }

    auto level_init_pre_entity() -> void override { Log::msg("init_pre_entity"); }
    auto level_init_post_entity() -> void override { Log::msg("level_init_post_entity"); }
    auto level_shutdown_pre_clear_steam_api_context() -> void override { Log::msg("level_shutdown_pre_clear_steam_api_context"); }
    auto level_shutdown_pre_entity() -> void override { Log::msg("level_shutdown_pre_entity"); }
    auto level_shutdown_post_entity() -> void override { Log::msg("level_shutdown_post_entity"); }

    auto update(float frametime) -> void override {
        //Log::msg("%f: %f", IFace<EngineClient>()->get_last_timestamp(), frametime);
    }

    Blue_GameSystem() {
        GameSystem::add_all();
    }
};

Blue_GameSystem blue;