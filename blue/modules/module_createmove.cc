#include <stdafx.hh>

#include "module_createmove.hh"

#include "convar.hh"
#include "interface.hh"
#include "log.hh"
#include "sdk.hh"

#include "blue_module_list.hh"
#include "blue_target_list.hh"

#include "hooks.hh"

using namespace TF;
using namespace BlueTarget;

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

DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, create_move);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, create_move_pre_predict);

//DEFINE_LIST_CALL_FUNCTION_RECURSIVE(TargetList, BlueTarget::Invoke, is_valid_target);

template <typename Invoker>
static void check_visible(Entity *e) {
    bool         is_visible = false;
    Math::Vector pos;

#ifdef _DEBUG
    pos = Math::Vector::invalid();
#endif

    if (Invoker::invoke_visible_target(e, std::ref(pos), std::ref(is_visible)), is_visible == false) return;

    assert(pos != Math::Vector::invalid());

    Invoker::invoke_finished_target(BlueTarget::Target{e, pos});
}

// recursively deal with targets
template <typename Con>
static void think_targets() {
    using Invoker = typename BlueTarget::Invoke<Con::Head>;

    Invoker::invoke_flush_targets();

    for (auto e : IFace<EntList>()->get_range()) {
        if (e == nullptr) continue;

        bool is_valid = false;
        if (Invoker::invoke_valid_target(e, std::ref(is_valid)), is_valid == false) continue;

        // TODO: apply some threads here

        check_visible<Invoker>(e);
    }

    Invoker::invoke_sort_targets();

    if constexpr (std::is_same_v<typename Con::Tail, BlueList::Nil> == false) think_targets<Con::Tail>();
}

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd) {
    ModuleList_call_create_move_pre_predict(user_cmd);

    think_targets<TargetList>();

    ModuleList_call_create_move(user_cmd);
    return true;
}
