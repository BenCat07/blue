#pragma once

#include "types.hh"
#include "vfunc.hh"

#include "datatable.hh"
#include "entity.hh"

namespace TF {
class UserCmd {
    virtual ~UserCmd(){};

public:
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
    auto get_networkable() {
    }
    auto get_entity(u32 index) -> Entity * {
        return_virtual_func(get_entity, 3, 0, 0, 0, index);
    }

    auto get_max_entity() -> u32 {
        return_virtual_func(get_max_entity, 6, 0, 0, 0);
    }
};

class Input {
public:
    auto get_user_cmd(u32 sequence_number) -> UserCmd * {
        return_virtual_func(get_user_cmd, 8, 8, 8, 0, sequence_number);
    }
};
} // namespace TF
