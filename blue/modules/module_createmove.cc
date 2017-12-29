#include <stdafx.hh>

#include "module_createmove.hh"

#include "convar.hh"
#include "interface.hh"
#include "log.hh"
#include "sdk.hh"

#include "blue_module_list.hh"

#include "hooks.hh"

using namespace TF;

DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, create_move);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, create_move_pre_predict);

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd);

namespace {
Convar<bool> do_queue_packets{"blue_do_queue_packets", true, nullptr};

Hooks::HookFunction<ClientMode, 0> *create_move_hook;
} // namespace

void CreateMoveModule::level_startup() {
    Log::msg("CreateMoveModule::level_startup()");
    create_move_hook = new Hooks::HookFunction<ClientMode, 0>(IFace<ClientMode>().get(), 21, &hooked_create_move);
}
void CreateMoveModule::level_shutdown() {
    Log::msg("CreateMoveModule::level_shutdown()");
    delete create_move_hook;
    create_move_hook = nullptr;
}

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd) {
    ModuleList_call_create_move_pre_predict(user_cmd);
    ModuleList_call_create_move(user_cmd);
    return false;
}
