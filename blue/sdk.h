#pragma once

#include "types.h"
#include "vfunc.h"

#include "datatable.h"
#include "entity.h"

namespace TF {
class UserCmd {
    virtual ~UserCmd(){};
    int          command_number;
    int          tick_count;
    Math::Vector viewangles;
    float        forwardmove;
    float        sidemove;
    float        upmove;
    int          buttons;
    u8           impulse;
    int          weapon_select;
    int          weapon_subtype;
    int          random_seed;
    short        mouse_dx;
    short        mouse_dy;
    bool         has_been_predicted;
};

struct ClientClass {
    void *       create_fn;
    void *       create_event_fn; // Only called for event objects.
    const char * network_name;
    RecvTable *  recv_table;
    ClientClass *next;
    int          class_id; // Managed by the engine.
};

class Client {
public:
    auto get_all_classes() -> ClientClass * {
        return_virtual_func(get_all_classes, 8, 8, 8, 0);
    }
};

class Engine {
public:
    auto get_last_timestamp() -> float {
        return_virtual_func(get_last_timestamp, 15, 15, 15, 0);
    }

    auto time() -> float {
        return_virtual_func(time, 14, 14, 14, 0);
    }
};

class EntList {
public:
    auto get_entity() -> Entity * {
        return_virtual_func(get_entity, 3, 0, 0, 0);
    }

    auto get_max_entity() -> u32 {
        return_virtual_func(get_max_entity, 6, 0, 0, 0);
    }
};
} // namespace TF