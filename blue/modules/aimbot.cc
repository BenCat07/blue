#include <stdafx.hh>

#include "aimbot.hh"

#include "entity.hh"
#include "interface.hh"
#include "player.hh"
#include "sdk.hh"

#include "blue_target_list.hh"

#include <algorithm>
#include <execution>

using namespace TF;
using namespace BlueTarget;

namespace {
std::vector<Target> targets;
std::atomic_uint    next_index;

Player *local_player;
Entity *local_weapon;

int          local_team;
Math::Vector local_view;

} // namespace

inline auto clamp_angle(const Math::Vector &angles) {
    Math::Vector out;

    out.x = angles.x;
    out.y = angles.y;
    out.z = 0;

    while (out.x > 89.0f) out.x -= 180.0f;
    while (out.x < -89.0f) out.x += 180.0f;

    while (out.y > 180.0f) out.y -= 360.0f;
    while (out.y < -180.0f) out.y += 360.0f;

    assert(out.x < 89.0f && out.x > -89.0f);
    assert(out.y < 180.0f && out.y > -180.0f);

    return out;
}

auto Aimbot::update(float frametime) -> void {
}

auto Aimbot::create_move(TF::UserCmd *cmd) -> void {
    if (targets.size() > 0) {
        if (targets[0].first != nullptr) {
            Math::Vector delta      = targets[0].second - local_view;
            Math::Vector new_angles = delta.to_angle();

            new_angles      = clamp_angle(new_angles);
            cmd->viewangles = new_angles;
        }
    }
}

auto Aimbot::create_move_pre_predict(TF::UserCmd *cmd) -> void {
}

static auto visible(Entity *e, const Math::Vector &position) {
    ::Trace::TraceResult result;
    ::Trace::Ray         ray;
    ::Trace::Filter      f(local_player);

    ray.init(local_view, position);

    // TODO: projectile prediction checks
    IFace<TF::Trace>()->trace_ray(ray, 0x4200400B, &f, &result);

    if (result.entity == e) return true;

    return false;
}

static auto multipoint_internal(Entity *e, float granularity, const Math::Vector &centre, Math::Vector &min, const Math::Vector &max, Math::Vector &out) {
    // go from centre to centre min first
    for (float i = 0.0f; i <= 1.0f; i += granularity) {
        Math::Vector point = centre.lerp(min, i);

        if (visible(e, point)) {
            out = point;
            return true;
        }
    }

    // now from centre to max
    for (float i = 0.0f; i <= 1.0f; i += granularity) {
        Math::Vector point = centre.lerp(max, i);

        if (visible(e, point)) {
            out = point;
            return true;
        }
    }

    return false;
}

static auto multipoint(Entity *e, const Math::Vector &centre, const Math::Vector &min, const Math::Vector &max, Math::Vector &position_out) {
    // new multipoint begin
    float divisor = 5;

    if (divisor == 0) return false;

    float granularity = 1.0f / divisor;

    // Here we need to base our min + max around the centre of the hitbox as they are offsets

    Math::Vector centre_min_x = Math::Vector(Math::lerp(0.5, min.x, max.x), min.y, centre.z);
    Math::Vector centre_max_x = Math::Vector(Math::lerp(0.5, min.x, max.x), max.y, centre.z);

    Math::Vector centre_min_y = Math::Vector(min.x, Math::lerp(0.5, min.y, max.y), centre.z);
    Math::Vector centre_max_y = Math::Vector(max.x, Math::lerp(0.5, min.y, max.y), centre.z);

    IFace<DebugOverlay>()->add_line_overlay(centre_min_x, centre_max_x, 255, 0, 0, true, 0);
    IFace<DebugOverlay>()->add_line_overlay(centre_min_y, centre_max_y, 255, 0, 0, true, 0);

    if (multipoint_internal(e, granularity, centre, centre_min_x, centre_max_x, position_out) == true)
        return true;
    else if (multipoint_internal(e, granularity, centre, centre_min_y, centre_max_y, position_out) == true)
        return true;

    return false;
}

auto Aimbot::visible_target(Entity *e, Math::Vector &pos, bool &visible) -> void {
    // TODO: should entity have a to_player_nocheck() method
    // as we already know at this point that this is a player...
    auto player = e->to_player();

    Player::PlayerHitboxes hitboxes;
    u32                    hitboxes_count = player->hitboxes(&hitboxes, false);

    // check best hitbox first
    if (::visible(e, hitboxes.centre[0])) {
        pos     = hitboxes.centre[0];
        visible = true;
        return;
    } else if (multipoint(e, hitboxes.centre[0], hitboxes.min[0], hitboxes.max[0], pos)) {
        visible = true;
        return;
    }

    // TODO: choose hitbox based on current weapon
    for (u32 i = 0; i < hitboxes_count; i++) {
        if (::visible(e, hitboxes.centre[i])) {
            pos     = hitboxes.centre[i];
            visible = true;
            return;
        } else if (multipoint(e, hitboxes.centre[i], hitboxes.min[i], hitboxes.max[i], pos)) {
            visible = true;
            return;
        }
    }

    visible = false;
    return;
}

auto Aimbot::valid_target(Entity *e, bool &valid) -> void {
    if (e->is_valid() == false) return;

    if (auto player = e->to_player()) {
        if (player->alive() == false) return;
        if (local_team == player->team()) return;

        valid = true;
    }

    return;
}

auto Aimbot::flush_targets() -> void {
    next_index = 0;
    targets.clear();
    targets.resize(IFace<EntList>()->max_entity_index());

    // deal with some local data that we want to keep around

    local_player = Player::local();
    assert(local_player);

    local_weapon = local_player->active_weapon();

    local_team = local_player->team();
    local_view = local_player->view_position();
}

auto Aimbot::finished_target(Target t) -> void {
    assert(t.first);

    targets[next_index] = t;

    next_index += 1;
}

auto Aimbot::sort_targets() -> void {
    std::sort(std::execution::par_unseq, targets.begin(), targets.end(),
              [](const Target &a, const Target &b) {
                  return a.second.distance(local_view) < b.second.distance(local_view);
              });
}
