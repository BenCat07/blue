#include "stdafx.hh"

#include "gamesystem.hh"
#include "log.hh"

#include "hooks.hh"
#include "interface.hh"
#include "netvar.hh"
#include "player.hh"
#include "sdk.hh"
#include "vfunc.hh"

#include "convar.hh"

#include "blue_module.hh"
#include "blue_module_list.hh"

DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, update);

class Blue_Core : public GameSystem {
    bool inited = false;

public:
    auto init() -> bool override {
        Log::msg("init()");
        return true;
    }
    auto post_init() -> void override {
        Log::msg("post_init()");

        IFace<TF::Client>().set_from_interface("client", "VClient");
        IFace<TF::Engine>().set_from_interface("engine", "VEngineClient");
        IFace<TF::EntList>().set_from_interface("client", "VClientEntityList");
        IFace<TF::Input>().set_from_pointer(**reinterpret_cast<TF::Input ***>(
            VFunc::get_func<u8 *>(IFace<TF::Client>().get(), 15, 0) + 0x2));
        IFace<TF::Cvar>().set_from_interface("vstdlib", "VEngineCvar");
    }
    auto process_attach() -> void {
        Log::msg("process_attach()");

        // make sure that the netvars are initialised
        // becuase their dynamic initialiser could be after the
        // gamesystems one
        TF::Netvar::init_all();

        // register all convars now that we have the interfaces we need
        Convar_Base::init_all();

        // at this point we are now inited and ready to go!
        inited = true;
    }

    auto shutdown() -> void override { Log::msg("shutdown()"); }

    auto level_init_pre_entity() -> void override { Log::msg("init_pre_entity()"); }
    auto level_init_post_entity() -> void override { Log::msg("level_init_post_entity"); }
    auto level_shutdown_pre_clear_steam_api_context() -> void override { Log::msg("level_shutdown_pre_clear_steam_api_context"); }
    auto level_shutdown_pre_entity() -> void override { Log::msg("level_shutdown_pre_entity"); }
    auto level_shutdown_post_entity() -> void override { Log::msg("level_shutdown_post_entity"); }

    // update is called from CHLClient_HudUpdate
    // in theory we should be able to render here
    // and be perfectly ok
    // HOWEVER: it might be better to do this at frame_end()
    auto update([[maybe_unused]] float frametime) -> void override {
        if (inited != true || IFace<TF::Engine>()->in_game() != true) return;

        if (auto *local = TF::Player::local()) {
            auto origin = local->origin();
            Log::msg("(%f, %f, %f)", origin.x, origin.y, origin.z);
        }

        ModuleList_call_update(frametime);
    }

    Blue_Core() {
        GameSystem::add_all();
    }
};

Convar<int> blue_test{"blue_test", 10, 0, 100, nullptr};

Blue_Core blue;

auto __stdcall blue_gamesystem_send_process_attach([[maybe_unused]] void *hmodule) -> u32 {
    // TODO: pass module over to the gamesystem
    blue.process_attach();

    return 0;
}
