#include <stdafx.hh>

#include "backtrack.hh"

#include "interface.hh"
#include "log.hh"
#include "netvar.hh"
#include "player.hh"
#include "sdk.hh"

#include <algorithm>

using namespace TF;

namespace {
enum {
    backtrack_max_anim_layers = 15,
};

// Storage for a tick
class BacktrackRecord {
public:
    // animation related
    std::array<AnimationLayer, backtrack_max_anim_layers> animation_layers;
    std::array<float, 15>                                 pose_parameters;
    float                                                 master_cycle;
    int                                                   master_sequence;

    // state related
    bool alive;
    bool origin_changed;
    bool angles_changed;
    bool size_changed;

    // absolute related
    Math::Vector origin;
    Math::Vector render_origin;
    Math::Vector angles;
    Math::Vector real_angles;
    Math::Vector min_prescaled;
    Math::Vector max_prescaled;

    PlayerHitboxes hitboxes;
    u32            max_hitboxes;

    // misc
    float   simulation_time;
    float   animation_time;
    float   choked_time;
    int     choked_ticks;
    UserCmd user_cmd;
    int     this_tick;

    BacktrackRecord() {
    }

    BacktrackRecord(const BacktrackRecord &other) { std::memcpy(this, &other, sizeof(BacktrackRecord)); }
    auto operator=(const BacktrackRecord &other) { std::memcpy(this, &other, sizeof(BacktrackRecord)); }

    // TODO: this is a nasty hack becuase hitbox registeration will happen before createmove gets called
    // This can be done better and should be done better
    auto reset() {
        std::memset(this, 0, sizeof(BacktrackRecord));
    }
};

enum {
    backtrack_max_players = 33,
};

auto tick_to_index(u32 tick) { return tick % Backtrack::max_ticks; }
auto entity_index_to_array_index(u32 index) { return index - 1; }

std::array<std::array<BacktrackRecord, Backtrack::max_ticks>, backtrack_max_players> backtrack_records;

auto &record_track(u32 index) { return backtrack_records[index]; }
auto &record(u32 index, u32 tick) { return record_track(index)[tick_to_index(tick)]; }

u32   current_tick = 0;
auto &current_record(u32 index) { return record(index, current_tick); }

std::vector<TF::Player *> players_to_restore;
} // namespace

auto Backtrack::level_startup() -> void {
}

auto Backtrack::level_shutdown() -> void {
}

auto Backtrack::create_move_pre_predict(UserCmd *cmd) -> void {
    // This can be called anywhere in the createmove function
    // Because either modules can use the current up to date data
    // or they will backtrack to a previous tick that isnt this current one.
    current_tick = IFace<Globals>()->tickcount;

    auto local_player = Player::local();

    auto real_tick = IFace<Globals>()->tickcount;

    for (auto entity : IFace<EntList>()->get_range(IFace<Engine>()->max_clients())) {
        if (entity == nullptr) continue;

        auto player = entity->to_player();
        if (player == nullptr || player == local_player) continue;

        if (player->team() == local_player->team()) continue;

        // Get the record for this tick and clear it out
        auto  array_index     = entity_index_to_array_index(entity->index());
        auto &new_record      = record(array_index, real_tick);
        auto &previous_record = record(array_index, real_tick - 1);

        // Clean out the record - might not be necessary but a memset is pretty cheap
        new_record.reset();

        new_record.alive = player->alive();

        // If this fails then it is clear to other algorithms whether this record is valid just by looking at this bool
        if (new_record.alive == false) continue;

        // Set absolute origin and angles
        new_record.origin        = player->origin();
        new_record.render_origin = player->render_origin();
        new_record.angles        = player->angles();

        // TODO: when real angles and corrected angles differ we need to change this
        new_record.real_angles = player->angles();

        // TODO: min_prescaled, max_prescaled

        new_record.origin_changed = previous_record.origin != new_record.origin;
        new_record.angles_changed = previous_record.angles != new_record.angles;
        new_record.size_changed   = previous_record.min_prescaled != new_record.min_prescaled ||
                                  previous_record.max_prescaled != new_record.max_prescaled;

        new_record.simulation_time = player->sim_time();
        new_record.animation_time  = player->anim_time();

        // TODO: calculate choked ticks...

        new_record.master_cycle    = player->cycle();
        new_record.master_sequence = player->sequence();
        new_record.this_tick       = real_tick;

        auto layer_count = player->anim_layer_count();
#ifdef _DEBUG
        if (layer_count > backtrack_max_anim_layers)
            Log::msg("Not enough space for all layers (has %d, needs %d)",
                     backtrack_max_anim_layers, layer_count);
#endif

        auto layer_max = std::min<u32>(backtrack_max_anim_layers, layer_count);
        for (u32 i = 0; i < layer_max; ++i) {
            // Isnt our animationlayer exactly the same as the games....
            auto  layer        = player->anim_layer(i);
            auto &layer_record = new_record.animation_layers[i];

            // TODO: memcpy?
            layer_record.sequence = layer.sequence;
            layer_record.cycle    = layer.cycle;
            layer_record.weight   = layer.weight;
            layer_record.order    = layer.order;

            layer_record.layer_anim_time = layer.layer_anim_time;
            layer_record.layer_fade_time = layer.layer_fade_time;

            layer_record.blend_in  = layer.blend_in;
            layer_record.blend_out = layer.blend_out;

            layer_record.client_blend = layer.client_blend;

            layer_record.playback_rate = layer.playback_rate;
        }

        // TODO: pose parameters (are these even important??)
    }
}

// TODO: remove
static Math::Vector hullcolor[8] =
    {
        Math::Vector(1.0, 1.0, 1.0),
        Math::Vector(1.0, 0.5, 0.5),
        Math::Vector(0.5, 1.0, 0.5),
        Math::Vector(1.0, 1.0, 0.5),
        Math::Vector(0.5, 0.5, 1.0),
        Math::Vector(1.0, 0.5, 1.0),
        Math::Vector(0.5, 1.0, 1.0),
        Math::Vector(1.0, 1.0, 1.0)};

auto Backtrack::create_move(TF::UserCmd *cmd) -> void {
    for (auto entity : IFace<EntList>()->get_range()) {
        if (entity->is_valid() == false) return;
        auto player = entity->to_player();
        if (player == nullptr) return;

        auto &record_track = ::record_track(entity_index_to_array_index(player->index()));

        for (auto &record : record_track) {
            IFace<DebugOverlay>()->add_text_overlay(record.origin, 0, "%d", record.this_tick);

            /*
            if (record.this_tick == cmd->tick_count)
                for (u32 i = 0; i < record.max_hitboxes; i++) {
                    auto &hitboxes = record.hitboxes;

                    int j = (record.this_tick % 8);
                    int r = (int)(255.0f * hullcolor[j].x);
                    int g = (int)(255.0f * hullcolor[j].y);
                    int b = (int)(255.0f * hullcolor[j].z);

                    IFace<DebugOverlay>()->add_box_overlay(hitboxes.origin[i], hitboxes.raw_min[i], hitboxes.raw_max[i], hitboxes.rotation[i], r, g, b, 100, 0);
                }
			*/
        }
    }
}

// This currently doesnt update tracerays against this player...
// We need to find another method of doing this (FSN isnt going to work either.)

static auto restore_player_to_record(TF::Player *p, const BacktrackRecord &r) {
    p->set_origin(r.origin);
    p->render_origin() = r.render_origin;
    p->set_angles(r.angles);

    p->sim_time()  = r.simulation_time;
    p->anim_time() = r.animation_time;

    p->cycle()    = r.master_cycle;
    p->sequence() = r.master_sequence;

    auto layer_count = p->anim_layer_count();
    auto layer_max   = std::min<u32>(backtrack_max_anim_layers, layer_count);

    for (u32 i = 0; i < layer_max; ++i) {
        auto        layer        = p->anim_layer(i);
        const auto &layer_record = r.animation_layers[i];

        layer.sequence = layer_record.sequence;
        layer.cycle    = layer_record.cycle;
        layer.weight   = layer_record.weight;
        layer.order    = layer_record.order;

        layer.layer_anim_time = layer_record.layer_anim_time;
        layer.layer_fade_time = layer_record.layer_fade_time;

        layer.blend_in     = layer_record.blend_in;
        layer.blend_out    = layer_record.blend_out;
        layer.client_blend = layer_record.client_blend;

        layer.playback_rate = layer_record.playback_rate;
    }

    // Update the clientside animation state
    p->update_client_side_animation();

    // Make sure to clean out the bone cache so that setupbones returns new values
#if blueplatform_windows()
    static auto g_iModelBoneCounter = *(Signature::find_pattern<long **>("client", "A1 ? ? ? ? D9 45 0C", 1));
    assert(g_iModelBoneCounter);
    (*g_iModelBoneCounter)++;
#endif

    return true;
}

auto Backtrack::backtrack_player_to_tick(TF::Player *p, u32 tick, bool restoring) -> bool {
    auto array_index = entity_index_to_array_index(p->index());

    auto &r = record(array_index, tick);
    if (r.alive == false) return false;

    if (restoring == false) players_to_restore.push_back(p);
    return restore_player_to_record(p, r);
}

auto Backtrack::create_move_finish(TF::UserCmd *cmd) -> void {
    for (auto &p : players_to_restore) {
        backtrack_player_to_tick(p, current_tick, true);
    }

    players_to_restore.clear();
}

auto Backtrack::update_player_hitboxes(Player *p, TF::PlayerHitboxes *h, u32 max_hitboxes) -> void {
    auto &record = current_record(entity_index_to_array_index(p->index()));

    std::memcpy(&record.hitboxes, h, sizeof(PlayerHitboxes));
    record.max_hitboxes = max_hitboxes;
}

auto Backtrack::get_hitboxes_for_player_at_tick(TF::Player *p, u32 tick, PlayerHitboxes *h) -> void {
    auto array_index = entity_index_to_array_index(p->index());

    auto &r = record(array_index, tick);

    if (r.alive == false) return;

    std::memcpy(h, &r.hitboxes, sizeof(PlayerHitboxes));
}
