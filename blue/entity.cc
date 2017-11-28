#include "stdafx.hh"

#include "entity.hh"

#include "netvar.hh"

using namespace TF;

auto Entity::is_valid() -> bool {
    // get around clangs "correctly formed code never
    //  has a null thisptr"
    auto thisptr = reinterpret_cast<uptr>(this);
    if (thisptr == 0) {
        return false;
    }

    return true;
}

static auto health = Netvar("DT_BasePlayer", "m_iHealth");
auto        Entity::health() -> int & {
    return ::health.get<int>(this);
}
