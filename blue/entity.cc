#include "stdafx.hh"

#include "entity.hh"
#include "player.hh"

#include "netvar.hh"

#include "sdk.hh"

using namespace TF;

auto Entity::is_valid() -> bool {
    // get around clangs "correctly formed code never
    //  has a null thisptr"
    auto thisptr = reinterpret_cast<uptr>(this);
    if (thisptr == 0) return false;

    return true;
}

auto Entity::to_player() -> class Player * {
    auto clientclass = client_class();

    // TODO: do not hardcode this value
    if (clientclass->class_id == 246) return static_cast<class Player *>(this);

    return nullptr;
}

auto Entity::client_class() -> struct ClientClass * {
    return_virtual_func(client_class, 2, 0, 0, 8);
}
