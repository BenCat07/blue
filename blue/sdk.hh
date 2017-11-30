#pragma once

#include "types.hh"
#include "vfunc.hh"

#include "datatable.hh"
#include "entity.hh"
#include "interface.hh"

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
    Engine() = delete;

    auto get_last_timestamp() -> float {
        return_virtual_func(get_last_timestamp, 15, 15, 15, 0);
    }

    auto time() -> float {
        return_virtual_func(time, 14, 14, 14, 0);
    }

    auto in_game() -> bool {
        return_virtual_func(in_game, 26, 26, 26, 0);
    }

    auto local_player_index() -> u32 {
        return_virtual_func(local_player_index, 12, 12, 12, 0);
    }
};

class EntList {
public:
    EntList() = delete;

    auto get_entity(u32 index) -> Entity * {
        return_virtual_func(get_entity, 3, 0, 0, 0, index);
    }

    auto get_max_entity() -> u32 {
        return_virtual_func(get_max_entity, 6, 0, 0, 0);
    }

    class EntityRange {
        EntList *parent;

    public:
        class Iterator {
            u32      index;
            EntList *parent;

        public:
            Iterator(EntList *parent) : index(0), parent(parent) {}
            explicit Iterator(u32 index, EntList *parent)
                : index(index), parent(parent) {}

            auto &operator++() {
                ++index;
                return *this;
            }

            auto operator*() {
                return parent->get_entity(index);
            }

            auto operator==(const Iterator &b) {
                return index == b.index;
            }

            auto operator!=(const Iterator &b) {
                return !(*this == b);
            }
        };

        EntityRange(EntList *parent) : parent(parent) {}

        auto begin() {
            return Iterator(parent);
        }

        auto end() {
            return Iterator(parent->get_max_entity(), parent);
        }
    };

    auto get_range() {
        return EntityRange(this);
    }
};

class Input {
public:
    Input() = delete;

    auto get_user_cmd(u32 sequence_number) -> UserCmd * {
        return_virtual_func(get_user_cmd, 8, 8, 8, 0, sequence_number);
    }
};
} // namespace TF
