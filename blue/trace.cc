#include "trace.hh"
#include "stdafx.hh"

#include "sdk.hh"

auto Trace::Filter::should_hit_entity(TF::Entity *handle_entity, int contents_mask) -> bool {
    auto handle      = handle_entity->to_handle();
    auto real_entity = IFace<TF::EntList>()->from_handle(handle);

    if (real_entity == nullptr) return false;

    auto client_class = real_entity->client_class();

    // ignore "bad" entities
    switch (client_class->class_id) {
    case 64:  // CFuncRespawnRoomVisualizer
    case 230: // CTFMedigunShield
    case 55:  // CFuncAreaPortalWindow
        return false;
    }

    if (real_entity == ignore_self ||
        real_entity == ignore_entity) return false;

    return true;
}
