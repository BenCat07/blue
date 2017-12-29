#pragma once

#include "types.hh"
#include "vfunc.hh"

#include "datatable.hh"
#include "entity.hh"
#include "interface.hh"
#include "signature.hh"

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
    Client() = delete;

    auto get_all_classes() -> ClientClass * {
        return_virtual_func(get_all_classes, 8, 8, 8, 0);
    }
};

class ClientMode {
public:
    ClientMode() = delete;
};

class NetChannel {
public:
    enum class Flow {
        outgoing,
        incoming
    };

    NetChannel() = delete;

    auto get_sequence_number(TF::NetChannel::Flow f) -> i32 {
        return_virtual_func(get_sequence_number, 16, 16, 16, 0, f);
    }

    auto queued_packets() -> i32 & {
        static auto queued_packets_offset = []() {
            if constexpr (BluePlatform::windows()) {
                return *Signature::find_pattern<u32 *>("engine", "83 BE ? ? ? ? ? 0F 9F C0 84 C0", 2);
            } else if constexpr (BluePlatform::linux()) {
                static_assert(BluePlatform::linux() == false);
            } else if constexpr (BluePlatform::osx()) {
                static_assert(BluePlatform::osx() == false);
            }
        }();

        assert(queued_packets_offset);

        auto &queued_packets = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + queued_packets_offset);

        return queued_packets;
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

    auto net_channel_info() -> NetChannel * {
        return_virtual_func(net_channel_info, 72, 72, 72, 0);
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
        static auto array_offset = []() {
            if constexpr (BluePlatform::windows()) {
                return *Signature::find_pattern<u32 *>("client", "8B 87 ? ? ? ? 8B CA", 2);
            } else if constexpr (BluePlatform::linux()) {
                static_assert(BluePlatform::linux() == false);
            } else if constexpr (BluePlatform::osx()) {
                static_assert(BluePlatform::osx() == false);
            }
        }();
        // this should not be 0
        assert(array_offset != 0);

        auto cmd_array = *reinterpret_cast<UserCmd **>(reinterpret_cast<u8 *>(this) + array_offset);

        return &cmd_array[sequence_number % 90];
    }

    auto get_verified_user_cmd(u32 sequence_number) -> class VerifiedCmd * {
        // 03 B7 ? ? ? ? 8D 04 88
    }
};

// defined in convar.cc
class ConCommandBase;

class Cvar {
public:
    Cvar() = delete;

    auto allocate_dll_identifier() -> u32 {
        return_virtual_func(allocate_dll_identifier, 5, 5, 5, 0);
    }

    auto register_command(ConCommandBase *command) -> void {
        return_virtual_func(register_command, 6, 6, 6, 0, command);
    }

    auto unregister_command(ConCommandBase *command) -> void {
        return_virtual_func(unregister_command, 7, 7, 7, 0, command);
    }
};

} // namespace TF
