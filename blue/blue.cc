#include "stdafx.hh"

#include "gamesystem.hh"
#include "log.hh"

#include "class_id.hh"
#include "hooks.hh"
#include "interface.hh"
#include "netvar.hh"
#include "player.hh"
#include "sdk.hh"
#include "vfunc.hh"

#include "convar.hh"

#include "modules/blue_module_list.hh"

DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, update);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, level_shutdown);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, level_startup);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, init_all);

class BlueCore : public GameSystem {
    bool inited = false;

    u32 old_sequence = 0;

public:
    auto init() -> bool override {
        // Guard against having init() called by the game and our constructor
        static bool init_happened = false;
        if (init_happened) return true;

#ifdef _DEBUG
            // create a debug console so that we can see the results
            // of all those lovely asserts
            //assert(AllocConsole() != 0);
            //freopen("CONOUT$", "w", stdout);
            //freopen("CONOUT$", "w", stderr);
#endif

        Log::msg("init()");
        init_happened = true;
        return true;
    }
    auto post_init() -> void override {
        Log::msg("post_init()");

        // Get interfaces here before init_all has a chance to do anything

        IFace<TF::Client>().set_from_interface("client", "VClient");
        IFace<TF::Engine>().set_from_interface("engine", "VEngineClient");
        IFace<TF::EntList>().set_from_interface("client", "VClientEntityList");
        IFace<TF::Input>().set_from_pointer(**reinterpret_cast<TF::Input ***>(
            VFunc::get_func<u8 *>(IFace<TF::Client>().get(), 15, 0) + 0x2));
        IFace<TF::Cvar>().set_from_interface("vstdlib", "VEngineCvar");
        IFace<TF::ClientMode>().set_from_pointer(
            *Signature::find_pattern<TF::ClientMode **>("client", "B9 ? ? ? ? A3 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 04 C7 05", 1));
        IFace<TF::ModelInfo>().set_from_interface("engine", "VModelInfoClient");
        IFace<TF::Trace>().set_from_interface("engine", "EngineTraceClient");
        IFace<TF::DebugOverlay>().set_from_interface("engine", "VDebugOverlay");
        IFace<TF::PlayerInfoManager>().set_from_interface("server", "PlayerInfoManager");

        IFace<TF::Globals>().set_from_pointer(IFace<TF::PlayerInfoManager>()->globals());

        auto globals_server_address = (u32)IFace<TF::Globals>().get();
        auto globals_real_address   = (u32)*Signature::find_pattern<TF::Globals **>("engine", "A1 ? ? ? ? 8B 11 68", 8);
        IFace<TF::Globals>().set_from_pointer((TF::Globals *)globals_real_address);

        IFace<TF::GameMovement>().set_from_interface("client", "GameMovement");
        IFace<TF::Prediction>().set_from_interface("client", "VClientPrediction");
    }
    auto process_attach() -> void {
        Log::msg("process_attach()");

        // TODO: should these two be modules??

        // make sure that the netvars are initialised
        // becuase their dynamic initialiser could be after the
        // gamesystems one
        TF::Netvar::init_all();

        // register all convars now that we have the interfaces we need
        Convar_Base::init_all();

        // call our modules init
        ModuleList_call_init_all();

        // at this point we are now inited and ready to go!
        inited = true;
    }

    auto shutdown() -> void override {
        // TODO: shutdown_all()

        Log::msg("shutdown()");
    }

    auto level_init_pre_entity() -> void override { Log::msg("init_pre_entity()"); }
    auto level_init_post_entity() -> void override {
        Log::msg("level_init_post_entity");

        // Make sure that all our class_ids are correct
        // This will only do anything on debug builds and not on release builds.

        // This needs to be done here becuase classids arent initialised before we are in game
        TF::ClassID::InternalChecker::ClassIDChecker::check_all_correct();

        ModuleList_call_level_startup();
    }
    auto level_shutdown_pre_clear_steam_api_context() -> void override { Log::msg("level_shutdown_pre_clear_steam_api_context"); }
    auto level_shutdown_pre_entity() -> void override {
        Log::msg("level_shutdown_pre_entity");
        ModuleList_call_level_shutdown();
    }
    auto level_shutdown_post_entity() -> void override { Log::msg("level_shutdown_post_entity"); }

    // update is called from CHLClient_HudUpdate
    // in theory we should be able to render here
    // and be perfectly ok
    // HOWEVER: it might be better to do this at frame_end()
    auto update([[maybe_unused]] float frametime) -> void override {
        if (inited != true || IFace<TF::Engine>()->in_game() != true) return;

        ModuleList_call_update(frametime);
    }

    BlueCore() {
        GameSystem::add_all();
    }
};

Convar<int> blue_test{"blue_test", 10, 0, 100, nullptr};

BlueCore blue;

auto __stdcall blue_gamesystem_send_process_attach([[maybe_unused]] void *hmodule) -> u32 {
    // TODO: pass module over to the gamesystem
    blue.process_attach();

    return 0;
}
