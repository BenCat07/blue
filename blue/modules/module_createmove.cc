#include <stdafx.hh>

#include "module_createmove.hh"

#include "convar.hh"
#include "interface.hh"
#include "log.hh"
#include "player.hh"
#include "sdk.hh"

#include "blue_module_list.hh"
#include "blue_target_list.hh"
#include "blue_threadpool.hh"

#include "hooks.hh"

using namespace TF;
using namespace BlueTarget;

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd);

namespace {

Hooks::HookFunction<ClientMode, 0> *create_move_hook = nullptr;

BlueThreading::ThreadPool *target_threadpool = nullptr;
// pointer to array of 2048 futures
std::vector<BlueThreading::ThreadPool::TaskFuture<void>> target_futures;
} // namespace

auto CreateMoveModule::level_startup() -> void {
    Log::msg("CreateMoveModule::level_startup()");

    Log::msg("=> Reserving memory");
    target_futures.reserve(2048);

    Log::msg("=> creating threadpool");
    assert(target_threadpool == nullptr);
    target_threadpool = new BlueThreading::ThreadPool;

    Log::msg("=> Hooking up!");
    assert(create_move_hook == nullptr);
    create_move_hook = new Hooks::HookFunction<ClientMode, 0>(IFace<ClientMode>().get(), 21, &hooked_create_move);
}
auto CreateMoveModule::level_shutdown() -> void {
    Log::msg("CreateMoveModule::level_shutdown()");

    Log::msg("<= Deleting hooks");
    delete create_move_hook;
    create_move_hook = nullptr;

    Log::msg("<= Deleting threadpool");
    delete target_threadpool;
    target_threadpool = nullptr;
}

DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, create_move);
DEFINE_LIST_CALL_FUNCTION_RECURSIVE(ModuleList, BlueModule::Invoke, create_move_pre_predict);

//DEFINE_LIST_CALL_FUNCTION_RECURSIVE(TargetList, BlueTarget::Invoke, is_valid_target);

template <typename Invoker>
static auto check_visible(Entity *e) {
    bool         is_visible = false;
    Math::Vector pos;

#ifdef _DEBUG
    pos = Math::Vector::invalid();
#endif

    if (Invoker::invoke_visible_target(e, std::ref(pos), std::ref(is_visible)), is_visible == false) return;

    assert(pos != Math::Vector::invalid());

    Invoker::invoke_finished_target(BlueTarget::Target{e, pos});
}

template <typename Con>
static auto think_target_recursive(Entity *e) {
    using Invoker = BlueTarget::Invoke<typename Con::Head>;

    bool is_valid = false;
    if (Invoker::invoke_valid_target(e, std::ref(is_valid)), is_valid == false) return;

    target_futures.push_back(target_threadpool->submit(&check_visible<Invoker>, e));

    if constexpr (std::is_same_v<typename Con::Tail, BlueList::Nil> == false) think_target_recursive<Con::Tail>();
}

template <typename Con>
static auto flush_targets_recursive() {
    using Invoker = BlueTarget::Invoke<typename Con::Head>;

    Invoker::invoke_flush_targets();

    if constexpr (std::is_same_v<typename Con::Tail, BlueList::Nil> == false) flush_targets_recursive<Con::Tail>();
}

template <typename Con>
static auto sort_targets_recursive() {
    using Invoker = BlueTarget::Invoke<typename Con::Head>;

    Invoker::invoke_sort_targets();

    if constexpr (std::is_same_v<typename Con::Tail, BlueList::Nil> == false) sort_targets_recursive<Con::Tail>();
}

// TODO: maybe there is a more efficient algorithm for doing this
// like sorting the entities first and then doing the calulations on them

// recursively deal with targets
// TODO: this is going to run the entity loop multiple times
// for multiple target handlers... which will waste alot of cycles
template <typename Con>
static void think_targets() {
    target_futures.clear();
    flush_targets_recursive<Con>();

    u32 futures_index = 0;
    for (auto e : IFace<EntList>()->get_range()) {
        if (e == nullptr) continue;
        if (e->dormant() == true) continue;

        think_target_recursive<Con>(e);
    }

    // Wait for all threads to finish
    for (auto &f : target_futures) f.get();

    sort_targets_recursive<Con>();
}

static auto local_player_prediction(Player *local, UserCmd *cmd) {
    char move_data_buffer[512];

    // Setup prediction
    auto old_cur_time   = IFace<Globals>()->curtime;
    auto old_frame_time = IFace<Globals>()->frametime;
    auto old_tick_count = IFace<Globals>()->tickcount;

    IFace<Globals>()->curtime   = local->tick_base() * IFace<Globals>()->interval_per_tick;
    IFace<Globals>()->frametime = IFace<Globals>()->interval_per_tick;
    IFace<Globals>()->tickcount = local->tick_base();

    // Set the current usercmd and run prediction
    local->set<UserCmd *, 0x107C>(cmd);

    IFace<Prediction>()->setup_move(local, cmd, nullptr, move_data_buffer);
    IFace<GameMovement>()->process_movement(local, move_data_buffer);
    IFace<Prediction>()->finish_move(local, cmd, move_data_buffer);

    local->set<UserCmd *, 0x107C>(0);

    // Cleanup from prediction
    IFace<Globals>()->curtime   = old_cur_time;
    IFace<Globals>()->frametime = old_frame_time;
    IFace<Globals>()->tickcount = old_tick_count;
}

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd) {
    auto local_player = Player::local();
    if (local_player == nullptr) return true;

    // Allow modules to look at the cmd before prediction modifies it
    ModuleList_call_create_move_pre_predict(user_cmd);

    local_player_prediction(local_player, user_cmd);

    think_targets<TargetList>();

    ModuleList_call_create_move(user_cmd);
    return false;
}
