#include <stdafx.hh>

#include "module_createmove.hh"

#include "convar.hh"
#include "interface.hh"
#include "log.hh"
#include "player.hh"
#include "sdk.hh"

#include "blue_threadpool.hh"

#include "hooks.hh"

using namespace TF;

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd);

//#define threaded_targetsystem

namespace {

Hooks::HookFunction<ClientMode, 0> *create_move_hook = nullptr;

#ifdef threaded_targetsystem
BlueThreading::ThreadPool *                              target_threadpool = nullptr;
std::vector<BlueThreading::ThreadPool::TaskFuture<void>> target_futures;
#endif
} // namespace

auto CreateMoveModule::level_startup() -> void {
    Log::msg("CreateMoveModule::level_startup()");

#ifdef threaded_targetsystem
    Log::msg("=> Reserving memory");
    target_futures.reserve(2048);

    Log::msg("=> creating threadpool");
    assert(target_threadpool == nullptr);
    target_threadpool = new BlueThreading::ThreadPool;
#endif

    Log::msg("=> Hooking up!");
    assert(create_move_hook == nullptr);
    create_move_hook = new Hooks::HookFunction<ClientMode, 0>(IFace<ClientMode>().get(), 21, &hooked_create_move);
}
auto CreateMoveModule::level_shutdown() -> void {
    Log::msg("CreateMoveModule::level_shutdown()");

    Log::msg("<= Deleting hooks");
    delete create_move_hook;
    create_move_hook = nullptr;

#ifdef threaded_targetsystem
    Log::msg("<= Deleting threadpool");
    delete target_threadpool;
    target_threadpool = nullptr;
#endif
}

// TODO: for visible targets we need some kind of phase based lookup
// For example, we want to check whether we have any visible people first
// and then try to do backtracking instead of doing backtracking before seeing
// whether we need to.

template <typename Invoker>
static auto check_visible(Entity *e) {
    bool         is_visible = false;
    Math::Vector pos;

#ifdef _DEBUG
    pos = Math::Vector::invalid();
#endif

    if (Invoker::visible_target(e, pos, is_visible), is_visible == false) return;

    assert(pos != Math::Vector::invalid());

    Invoker::finished_target(BlueTarget::Target{e, pos});
}

template <typename Invoker>
static auto think_target(Entity *e) {
    bool is_valid = false;
    Invoker::valid_target(e, std::ref(is_valid));

#ifdef threaded_targetsystem
    if (is_valid) target_futures.push_back(target_threadpool->submit(&check_visible<Invoker>, e));
#else
    if (is_valid) check_visible<Invoker>(e);
#endif
}

// TODO: maybe there is a more efficient algorithm for doing this
// like sorting the entities first and then doing the calulations on them

#include "aimbot.hh"

// recursively deal with targets
static void think_targets() {
#ifdef threaded_targetsystem
    target_futures.clear();
#endif
    Aimbot::flush_targets();

    for (auto e : IFace<EntList>()->get_range()) {
        if (e->is_valid() == false) continue;
        if (e->dormant() == true) continue;

        think_target<Aimbot>(e);
    }

#ifdef threaded_targetsystem
    // Wait for all threads to finish
    for (auto &f : target_futures) f.get();
#endif

    Aimbot::sort_targets();
}

// TODO: this isnt working correctly at all.
// Please fix.
static auto local_player_prediction(Player *local, UserCmd *cmd) {
    char move_data_buffer[512];
    memset(move_data_buffer, 0, sizeof(move_data_buffer));

    // Setup prediction
    auto old_cur_time   = IFace<Globals>()->curtime;
    auto old_frame_time = IFace<Globals>()->frametime;
    auto old_tick_count = IFace<Globals>()->tickcount;

    IFace<Globals>()->curtime = local->tick_base() * IFace<Globals>()->interval_per_tick;

    // If we are already not able to fit enough ticks into a frame account for this!!
    IFace<Globals>()->frametime = IFace<Globals>()->interval_per_tick;
    IFace<Globals>()->tickcount = local->tick_base();

    // Set the current usercmd and run prediction
    local->set<UserCmd *, 0x107C>(cmd);

    IFace<Prediction>()->setup_move(local, cmd, IFace<MoveHelper>().get(), move_data_buffer);
    IFace<GameMovement>()->process_movement(local, move_data_buffer);
    IFace<Prediction>()->finish_move(local, cmd, move_data_buffer);

    local->set<UserCmd *, 0x107C>(0);

    // Cleanup from prediction
    IFace<Globals>()->curtime   = old_cur_time;
    IFace<Globals>()->frametime = old_frame_time;
    IFace<Globals>()->tickcount = old_tick_count;
}

#include "backtrack.hh"

bool __fastcall hooked_create_move(void *instance, void *edx, float sample_framerate, UserCmd *user_cmd) {
    auto local_player = Player::local();
    if (local_player == nullptr) return true;

    // Allow modules to look at the cmd before prediction modifies it
    Aimbot::create_move_pre_predict(user_cmd);
    Backtrack::create_move_pre_predict(user_cmd);

    local_player_prediction(local_player, user_cmd);

    think_targets();

    Aimbot::create_move(user_cmd);
    Backtrack::create_move(user_cmd);

    Backtrack::create_move_finish(user_cmd);

    return false;
}
