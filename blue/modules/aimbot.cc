#include <stdafx.hh>

#include "aimbot.hh"

#include "backtrack.hh"
#include "class_id.hh"
#include "entity.hh"
#include "interface.hh"
#include "player.hh"
#include "sdk.hh"
#include "weapon.hh"

#include "blue_target_list.hh"
#include "convar.hh"

#include "blue_threadpool.hh"

#include "log.hh"
#include "math.hh"

#include <algorithm>
#include <execution>

using namespace TF;
using namespace BlueTarget;

namespace {
std::vector<Target> targets;
std::atomic_uint    next_index;

Player *local_player;
Weapon *local_weapon;

bool can_find_targets = false;

int          local_team;
Math::Vector local_view;

// TODO: HORRIBLE HACK
// this needs to be done on a per target basis and not like this !!!
u32 cmd_delta = 0;

} // namespace

// TODO: move this outside of aimbot
// Other modules may mess the angles up aswell
// So our best bet is to run this at the end of createmove...
inline static auto clamp_angle(const Math::Vector &angles) {
    Math::Vector out;

    out.x = angles.x;
    out.y = angles.y;
    out.z = 0;

    while (out.x > 89.0f) out.x -= 180.0f;
    while (out.x < -89.0f) out.x += 180.0f;

    while (out.y > 180.0f) out.y -= 360.0f;
    while (out.y < -180.0f) out.y += 360.0f;

    out.y = std::clamp(out.y, -180.0f, 180.0f);
    out.x = std::clamp(out.x, -90.0f, 90.0f);

    return out;
}

inline static auto fix_movement_for_new_angles(const Math::Vector &movement, const Math::Vector &old_angles, const Math::Vector &new_angles) {
    Math::Matrix3x4 rotate_matrix;

    auto delta_angles = new_angles - old_angles;
    delta_angles      = clamp_angle(delta_angles);

    rotate_matrix.from_angle(delta_angles);
    return rotate_matrix.rotate_vector(movement);
}

auto Aimbot::init_all() -> void {
}

auto Aimbot::update(float frametime) -> void {
}

// TODO: something something zoomed only convar
static auto try_autoshoot(TF::UserCmd *cmd) {
    auto autoshoot_allowed = false;

    // Only allow autoshoot when we are zoomed and can get headshots
    if (local_weapon->client_class()->class_id == ClassID::CTFSniperRifle && (local_player->cond() & 2)) {
        if ((local_player->tick_base() * IFace<Globals>()->interval_per_tick - local_player->fov_time()) >= 0.2)
            autoshoot_allowed = true;
    }

    if (autoshoot_allowed) cmd->buttons |= 1;
}

static auto blue_aimbot_silent                       = Convar<bool>{"blue_aimbot_silent", true, nullptr};
static auto blue_aimbot_autoshoot                    = Convar<bool>{"blue_aimbot_autoshoot", true, nullptr};
static auto blue_aimbot_aim_if_not_attack            = Convar<bool>{"blue_aimbot_aim_if_not_attack", true, nullptr};
static auto blue_aimbot_disallow_attack_if_no_target = Convar<bool>{"blue_aimbot_disallow_attack_if_no_target", false, nullptr};

auto Aimbot::create_move(TF::UserCmd *cmd) -> void {
    if (local_weapon == nullptr) return;

    if (blue_aimbot_aim_if_not_attack != true) {
        // check if we are IN_ATTACK
        if ((cmd->buttons & 1) != 1) return;
    }

    if (targets.size() > 0 && targets[0].first != nullptr) {
        Log::msg("[Aimbot] has target! (%d)", targets[0].first->index());
        IFace<DebugOverlay>()->add_box_overlay(targets[0].second, {-2, -2, -2}, {2, 2, 2}, {0, 0, 0}, 255, 255, 0, 100, 0);

        Math::Vector delta      = targets[0].second - local_view;
        Math::Vector new_angles = delta.to_angle();
        new_angles              = clamp_angle(new_angles);

        Math::Vector new_movement = fix_movement_for_new_angles({cmd->forwardmove, cmd->sidemove, 0}, cmd->viewangles, new_angles);

        if (local_weapon->can_shoot(local_player->tick_base())) {
            cmd->viewangles = new_angles;

            if (blue_aimbot_autoshoot == true) try_autoshoot(cmd);

            cmd->forwardmove = new_movement.x;
            cmd->sidemove    = new_movement.y;

            cmd->tick_count -= cmd_delta;
        }

        if (blue_aimbot_silent == false) IFace<Engine>()->set_view_angles(new_angles);
    } else {
        if (blue_aimbot_disallow_attack_if_no_target == true) cmd->buttons &= ~1;
    }
}

auto Aimbot::create_move_pre_predict(TF::UserCmd *cmd) -> void {
}

// Check it a point is visible to the player
static auto visible_no_entity(const Math::Vector &position) {
    ::Trace::TraceResult result;
    ::Trace::Ray         ray;
    ::Trace::Filter      f(local_player);

    ray.init(local_view, position);

    IFace<TF::Trace>()->trace_ray(ray, 0x46004003, &f, &result);

    return result.fraction == 1.0f;
}

static auto blue_aimbot_pedantic_mode = Convar<bool>{"blue_aimbot_pedantic_mode", true, nullptr};

static auto visible(Entity *e, const Math::Vector &position, const int hitbox) {
    ::Trace::TraceResult result;
    ::Trace::Ray         ray;
    ::Trace::Filter      f(local_player);

    ray.init(local_view, position);

    IFace<TF::Trace>()->trace_ray(ray, 0x46004003, &f, &result);

    //IFace<DebugOverlay>()->add_line_overlay(local_view, position, 0, 255, 0, true, -1.0f);
    //IFace<DebugOverlay>()->add_box_overlay(result.end_pos, {-2, -2, -2}, {2, 2, 2}, {0, 0, 0}, 255, 0, 0, 255, -1.0f);

    if (blue_aimbot_pedantic_mode == true) {
        if (result.entity == e && result.hitbox == hitbox) return true;
    } else if (result.entity == e) {
        return true;
    }

    return false;
}

static auto multipoint_internal(Entity *e, float granularity, const int hitbox, const Math::Vector &centre, const Math::Vector &min, const Math::Vector &max, Math::Vector &out) {
    // go from centre to centre min first
    for (float i = 0.0f; i <= 1.0f; i += granularity) {
        Math::Vector point = centre.lerp(min, i);

        if (visible(e, point, hitbox)) {
            out = point;
            return true;
        }
    }

    // now from centre to max
    for (float i = 0.0f; i <= 1.0f; i += granularity) {
        Math::Vector point = centre.lerp(max, i);

        if (visible(e, point, hitbox)) {
            out = point;
            return true;
        }
    }

    return false;
}

// TODO: remove
static auto blue_aimbot_multipoint_granularity = Convar<float>{"blue_aimbot_multipoint_granularity", 0, 0, 10, nullptr};

// TODO: there must be some kind of better conversion we can use here to get a straight line across the hitbox
static auto multipoint(Player *player, const int hitbox, const Math::Vector &centre, const Math::Vector &min, const Math::Vector &max, Math::Vector &position_out) -> bool {
    // create a divisor out of the granularity
    // TODO: precalculate??
    float divisor = blue_aimbot_multipoint_granularity;
    if (divisor == 0) return false;
    float granularity = 1.0f / divisor;

    auto new_x = Math::lerp(0.5, min.x, max.x);

    // Create a horizontal cross shape out of this box instead of top left bottom right or visa versa
    Math::Vector centre_min_x = Math::Vector(new_x, min.y, centre.z);
    Math::Vector centre_max_x = Math::Vector(new_x, max.y, centre.z);

    if (multipoint_internal(player, granularity, hitbox, centre, centre_min_x, centre_max_x, position_out) == true)
        return true;

    Math::Vector centre_min_y = Math::Vector(min.x, Math::lerp(0.5, min.y, max.y), centre.z);
    Math::Vector centre_max_y = Math::Vector(max.x, Math::lerp(0.5, min.y, max.y), centre.z);

    if (multipoint_internal(player, granularity, hitbox, centre, centre_min_y, centre_max_y, position_out) == true)
        return true;

    return false;
}

static auto find_best_box() {
    auto tf_class        = local_player->tf_class();
    auto weapon_class_id = local_weapon->client_class()->class_id;

    switch (tf_class) {
    case 2:                                                         // sniper
        if (weapon_class_id == 305) return std::make_pair(0, true); // aim head with the rifle
    default:
        return std::make_pair(3, false); // chest
    }
}

static auto blue_aimbot_enable_backtrack = Convar<bool>{"blue_aimbot_enable_backtrack", true, nullptr};

auto Aimbot::visible_target(Entity *e, Math::Vector &pos, bool &visible) -> void {
    // TODO: should entity have a to_player_nocheck() method
    // as we already know at this point that this is a player...
    auto player = e->to_player();

    PlayerHitboxes hitboxes;
    u32            hitboxes_count = player->hitboxes(&hitboxes, false);

    // Tell backtrack about these hitboxes
    Backtrack::update_player_hitboxes(player, &hitboxes, hitboxes_count);

    auto current_tick = IFace<Globals>()->tickcount;

    auto delta = 0;
    while (delta < Backtrack::max_ticks) {
        cmd_delta = delta;

        // TODO: there must be a better way to do this...
        if (delta > 0) Backtrack::get_hitboxes_for_player_at_tick(player, current_tick - delta, &hitboxes);

        auto best_box = find_best_box();

        // check best hitbox first
        if (::visible(e, hitboxes.centre[best_box.first], best_box.first)) {
            pos     = hitboxes.centre[best_box.first];
            visible = true;
            return;
        } else if (multipoint(player, best_box.first, hitboxes.centre[best_box.first], hitboxes.min[best_box.first], hitboxes.max[best_box.first], pos)) {
            visible = true;
            return;
        }

        // .second is whether we should only check the best box
        if (best_box.second != true) {
            for (u32 i = 0; i < hitboxes_count; i++) {
                if (::visible(e, hitboxes.centre[i], i)) {
                    pos     = hitboxes.centre[i];
                    visible = true;
                    return;
                }
            }

            // Perform multiboxing after confirming that we do not have any other options
            for (u32 i = 0; i < hitboxes_count; i++) {
                if (multipoint(player, i, hitboxes.centre[i], hitboxes.min[i], hitboxes.max[i], std::ref(pos))) {
                    visible = true;
                    return;
                }
            }
        }

        // If we dont want to do backtracking, escape
        if (blue_aimbot_enable_backtrack == false) break;

        // Backtrack to the previous tick
        // backtrack_player_to_tick will return false if the player is dead at this tick
        // so we need to keep trying until we hit the max or we find an alive state
        auto success = false;
        do {
            delta += 1;
            success = Backtrack::backtrack_player_to_tick(player, current_tick - delta);
        } while (success == false && delta < Backtrack::max_ticks);
    }

    visible = false;
    return;
}

auto Aimbot::valid_target(Entity *e, bool &valid) -> void {
    if (can_find_targets == false) {
        valid = false;
        return;
    }

    if (auto player = e->to_player()) {
        if (player->alive() == false) return;
        if (local_team == player->team()) return;

        IFace<DebugOverlay>()->add_entity_text_overlay(e->index(), 0, 0, 255, 255, 255, 255, "valid");
        IFace<DebugOverlay>()->add_entity_text_overlay(e->index(), 1, 0, 255, 255, 255, 255, "%d", player->index());

        valid = true;
    }

    return;
}

auto Aimbot::flush_targets() -> void {
    next_index = 0;

    // deal with some local data that we want to keep around
    local_player = Player::local();
    assert(local_player);

    local_weapon = local_player->active_weapon()->to_weapon();

    local_team = local_player->team();
    local_view = local_player->view_position();

    // If we dont have the necessary information (we havent spawned yet or are dead)
    // then do not attempt to find targets.
    can_find_targets = local_weapon != nullptr;

    targets.clear();
    targets.resize(IFace<EntList>()->max_entity_index());
}

auto Aimbot::finished_target(Target t) -> void {
    assert(t.first);

    IFace<DebugOverlay>()->add_entity_text_overlay(t.first->index(), 2, 0, 255, 255, 255, 255, "finished");

    targets[next_index] = t;

    next_index += 1;
}

auto Aimbot::sort_targets() -> void {
    std::sort(std::execution::seq, targets.begin(), targets.end(),
              [](const Target &a, const Target &b) {
                  return a.second.distance(local_view) < b.second.distance(local_view);
              });
}
