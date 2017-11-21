#include "stdafx.h"

#include "gamesystem.h"
#include "log.h"

#include "hooks.h"
#include "interface.h"
#include "netvar.h"
#include "sdk.h"
#include "vfunc.h"

class Blue_GameSystem : public GameSystem {
    bool inted = false;

public:
    auto init() -> bool override {
        Log::msg("init()");
        return true;
    }
    auto post_init() -> void override {
        Log::msg("post_init()");

        IFace<TF::Client>().set_interface("client", "VClient");

        inted = true;
    }
    auto process_attach() -> void {
        Log::msg("process_attach()");

        // make sure that the netvars are initialised
        // becuase their dynamic initialiser could be after the
        // gamesystems one
        TF::Netvar::init_all();
    }

    auto shutdown() -> void override { Log::msg("shutdown()"); }

    auto level_init_pre_entity() -> void override {
        Log::msg("init_pre_entity()");
    }
    auto level_init_post_entity() -> void override { Log::msg("level_init_post_entity"); }
    auto level_shutdown_pre_clear_steam_api_context() -> void override { Log::msg("level_shutdown_pre_clear_steam_api_context"); }
    auto level_shutdown_pre_entity() -> void override { Log::msg("level_shutdown_pre_entity"); }
    auto level_shutdown_post_entity() -> void override { Log::msg("level_shutdown_post_entity"); }

    // update is called from CHLClient_HudUpdate
    auto update(float frametime) -> void override {
    }

    Blue_GameSystem() {
        GameSystem::add_all();
    }
};

Blue_GameSystem blue;

auto __stdcall blue_gamesystem_send_process_attach(void *hmodule) {
    // TODO: pass module over to the gamesystem
    blue.process_attach();
}