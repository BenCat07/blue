#pragma once

#include "types.hh"
#include "vfunc.hh"

#include "datatable.hh"
#include "entity.hh"
#include "interface.hh"
#include "signature.hh"

#include "Trace.hh"

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

    auto last_timestamp() -> float {
        return_virtual_func(last_timestamp, 15, 15, 15, 0);
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

    auto entity(u32 index) -> Entity * {
        return_virtual_func(entity, 3, 0, 0, 0, index);
    }

    auto from_handle(EntityHandle h) -> Entity * {
        return_virtual_func(from_handle, 4, 4, 4, 0, h);
    }

    auto max_entity_index() -> u32 {
        return_virtual_func(max_entity_index, 6, 0, 0, 0);
    }

    class EntityRange {
        EntList *parent;

    public:
        class Iterator {
            u32      index;
            EntList *parent;

        public:
            // TODO: should we use 1 here or should we use 0 and force people
            // that loop to deal with that themselves...
            Iterator(EntList *parent) : index(1), parent(parent) {}
            explicit Iterator(u32 index, EntList *parent)
                : index(index), parent(parent) {}

            auto &operator++() {
                ++index;
                return *this;
            }

            auto operator*() {
                return parent->entity(index);
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
            return Iterator(parent->max_entity_index(), parent);
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

class Trace {
public:
    Trace() = delete;

    auto trace_ray(const ::Trace::Ray &ray, u32 mask, ::Trace::Filter *filter, ::Trace::TraceResult *results) -> void {
        return_virtual_func(trace_ray, 4, 4, 4, 0, ray, mask, filter, results);
    }
};

// These are defined to give us distinct types for these
// very different objects, to help us differentiate between them

class ModelHandle; // model_t

// mstudiobbox
class StudioHitbox {
public:
    StudioHitbox() = delete;

    int bone;
    int group;

    Math::Vector min;
    Math::Vector max;

    int name_index;

private:
    int unused[8];
};

// mstudiobboxset
class StudioHitboxSet {
public:
    StudioHitboxSet() = delete;

    int  name_index;
    auto name() const -> const char * {
        assert(0);
        return "";
    }

    u32 hitboxes_count;
    u32 hitbox_index;

    const auto operator[](u32 index) const {
        assert(index < hitboxes_count);

        return reinterpret_cast<const StudioHitbox *>(
                   reinterpret_cast<const u8 *>(this) + hitbox_index) +
               index;
    }
};

// studiohdr_t
class StudioModel {
public:
    StudioModel() = delete;

    int  id;
    int  version;
    int  checksum;
    char name[64];
    int  length;

    Math::Vector eye_position; // ideal eye position

    Math::Vector illumination_position; // illumination center

    Math::Vector hull_min; // ideal movement hull size
    Math::Vector hull_max;

    Math::Vector view_bbmin; // clipping bounding box
    Math::Vector view_bbmax;

    int flags;

    int bones_count;
    int bone_index;

    int bone_controllers_count;
    int bone_controller_index;

    int hitbox_sets_count;
    int hitbox_set_index;

    const auto hitbox_set(u32 index) const {
        assert(index < hitbox_sets_count);

        return reinterpret_cast<const StudioHitboxSet *>(
                   reinterpret_cast<const u8 *>(this) + hitbox_set_index) +
               index;
    }

    //... TODO:
};

class ModelInfo {
public:
    ModelInfo() = delete;

    auto model_name(const ModelHandle *m) -> const char * {
        return_virtual_func(model_name, 3, 0, 0, 0, m);
    }

    auto studio_model(const ModelHandle *m) -> const StudioModel * {
        return_virtual_func(studio_model, 28, 0, 0, 0, m);
    }
};

// This should only be used for debugging.
// For real output use the overlay
class DebugOverlay {
public:
    using OverlayText_t = void;

    virtual void add_entity_text_overlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...)                                                                         = 0;
    virtual void add_box_overlay(const Math::Vector &origin, const Math::Vector &mins, const Math::Vector &max, const Math ::Vector &orientation, int r, int g, int b, int a, float duration)                         = 0;
    virtual void add_triangle_overlay(const Math::Vector &p1, const Math::Vector &p2, const Math::Vector &p3, int r, int g, int b, int a, bool no_depth_test, float duration)                                         = 0;
    virtual void add_line_overlay(const Math::Vector &origin, const Math::Vector &dest, int r, int g, int b, bool no_depth_test, float duration)                                                                      = 0;
    virtual void add_text_overlay(const Math::Vector &origin, float duration, const char *format, ...)                                                                                                                = 0;
    virtual void add_text_overlay(const Math::Vector &origin, int line_offset, float duration, const char *format, ...)                                                                                               = 0;
    virtual void add_screen_text_overlay(float x_pos, float y_pos, float duration, int r, int g, int b, int a, const char *text)                                                                                      = 0;
    virtual void add_swept_box_overlay(const Math::Vector &start, const Math::Vector &end, const Math::Vector &mins, const Math::Vector &max, const Math::Vector &angles, int r, int g, int b, int a, float duration) = 0;
    virtual void add_grid_overlay(const Math::Vector &origin)                                                                                                                                                         = 0;
    virtual int  screen_position(const Math::Vector &point, Math::Vector &screen)                                                                                                                                     = 0;
    virtual int  screen_position(float x_pos, float y_pos, Math::Vector &screen)                                                                                                                                      = 0;

    virtual OverlayText_t *get_first(void)                  = 0;
    virtual OverlayText_t *get_next(OverlayText_t *current) = 0;
    virtual void           clear_dead_overlays()            = 0;
    virtual void           clear_all_overlays()             = 0;

    virtual void add_text_overlay_rgb(const Math::Vector &origin, int line_offset, float duration, float r, float g, float b, float alpha, const char *format, ...) = 0;
    virtual void add_text_overlay_rgb(const Math::Vector &origin, int line_offset, float duration, int r, int g, int b, int a, const char *format, ...)             = 0;

    virtual void add_line_overlay_alpha(const Math::Vector &origin, const Math::Vector &dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    //virtual void add_box_overlay2(const Math::Vector &origin, const Math::Vector &mins, const Math::Vector &max, const Math::Vector &orientation, const Color &faceColor, const Color &edgeColor, float duration) = 0;
};

} // namespace TF
