#include "stdafx.hh"

#include "player.hh"

#include "netvar.hh"
#include "vfunc.hh"

#include "sdk.hh"

using namespace TF;

auto Player::local() -> Player * {
    return static_cast<Player *>(IFace<EntList>()->get_entity(IFace<Engine>()->local_player_index()));
}

static auto health = Netvar("DT_BasePlayer", "m_iHealth");
auto        Player::health() -> int & {
    return ::health.get<int>(this);
}

static auto lifestate = Netvar("DT_BasePlayer", "m_lifeState");
auto        Player::alive() -> bool {
    return ::lifestate.get<int>(this) == 0;
}

auto Player::origin() -> Math::Vector & {
    return_virtual_func(origin, 9, 0, 0, 0);
}

auto Player::render_bounds() -> std::pair<Math::Vector, Math::Vector> {
    auto func = VFunc::Func<void (Player::*)(Math::Vector &, Math::Vector &)>(this, 20, 0, 0, 4);

    std::pair<Math::Vector, Math::Vector> ret;

    func.invoke(this, ret.first, ret.second);

    return ret;
}
