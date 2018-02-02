#include "stdafx.hh"

#include "convar.hh"

#include "interface.hh"
#include "log.hh"
#include "sdk.hh"

// implementation of a source convar
namespace TF {

class IConVar;

using ChangeCallbackFn = void (*)(IConVar *, const char *, float);

static auto dll_identifier = -1;

class ConCommandBase {
public:
    ConCommandBase() : name(nullptr), value_string(nullptr), help_string(nullptr) {}
    ConCommandBase(const char *name, const char *help_string, u32 flags = 0) {
        create_base(name, help_string, flags);
    }
    virtual ~ConCommandBase() {
        IFace<Cvar>()->unregister_command(this);
    }

    virtual auto is_command() -> bool const { return false; }

    virtual auto has_flag(int flag) -> bool const { return (flags & flag); }
    virtual auto add_flag(int new_flags) -> void const { flags |= new_flags; }

    virtual auto get_name() -> const char *const { return name; }
    virtual auto get_help_text() -> const char *const { return help_string; }

    virtual auto is_registered() -> bool const { return registered; }

    virtual auto get_dll_identifier() -> int const {
        if (dll_identifier == -1) dll_identifier = IFace<Cvar>()->allocate_dll_identifier();
        return dll_identifier;
    }

    virtual auto create_base(const char *name, const char *help_string, int flags = 0) -> void {
        assert(name);
        assert(help_string);

        registered = false;

        this->name        = name;
        this->help_string = help_string;
        this->flags       = flags;

        next = head;
        head = this;

        // We might not have Cvar here (global variables)

        if (auto cvar = IFace<Cvar>()) {
            IFace<Cvar>()->register_command(this);
        }
    }

    virtual auto init() -> bool {
        IFace<Cvar>()->register_command(this);
        return true;
    }

    // Convar vtable items
    virtual auto set_value(const char *value) -> void {
        assert(parent == this); // Only valid for root convars.

        float old_value = value_float;

        float new_value;
        if (value == nullptr)
            new_value = 0.0f;
        else
            new_value = (float)atof(value);

        // if we need to clamp this value then swap the original string
        // out with a temp one
        auto new_string_value = value;
        char temp_value[32];
        if (clamp_value(new_value) == true) {
            snprintf(temp_value, sizeof(temp_value), "%f", new_value);
            new_string_value = temp_value;
        }

        // Redetermine value
        value_float = new_value;
        value_int   = static_cast<int>(value_float);

        // TODO: do we need to handle never as string convars??
        change_string_value(new_string_value, old_value);
    }
    virtual auto set_value(float new_value) -> void {
        assert(parent == this);

        clamp_value(new_value);

        auto old_value = value_float;
        value_float    = new_value;
        value_int      = static_cast<int>(new_value);

        char temp_value[32];
        snprintf(temp_value, sizeof(temp_value), "%f", new_value);
        change_string_value(temp_value, old_value);
    }
    virtual auto set_value(int new_value) -> void {
        return set_value(static_cast<float>(new_value));
    }

    virtual auto internal_set_value(const char *new_value) -> void {
        return set_value(new_value);
    }
    virtual auto internal_set_value(float new_value) -> void {
        return set_value(new_value);
    }
    virtual auto internal_set_value(int new_value) -> void {
        return set_value(new_value);
    }

    virtual auto clamp_value(float &value) -> bool {
        if (has_min && (value < value_min)) {
            value = value_min;
            return true;
        }

        if (has_max && (value > value_max)) {
            value = value_max;
            return true;
        }

        return false;
    }

    virtual auto change_string_value(const char *new_value, float old_value) -> void {
        if (value_string != nullptr) {
            delete[] value_string;
            value_string = nullptr;
        }

        auto new_len = strlen(new_value) + 1;
        value_string = new char[new_len];
        strncpy(value_string, new_value, new_len);
        value_string_length = new_len;

        if (change_callback != nullptr) change_callback(to_iconvar(), new_value, old_value);
    }

    // helper functions for converting from IConVar and to IConVar
    static auto from_iconvar(IConVar *v) { return reinterpret_cast<ConCommandBase *>(reinterpret_cast<u8 *>(v) - 24); }
    static auto to_iconvar(ConCommandBase *b) { return reinterpret_cast<IConVar *>(reinterpret_cast<u8 *>(b) + 24); }
    auto        to_iconvar() -> IConVar * { return ConCommandBase::to_iconvar(this); }

#define DEFINE_THUNK(type, name, real_name)                              \
    static auto __fastcall name(IConVar *ecx, void *edx, type v)->void { \
        auto *real = ConCommandBase::from_iconvar(ecx);                  \
        real->real_name(v);                                              \
    }

    DEFINE_THUNK(const char *, set_value_string_thunk, set_value);
    DEFINE_THUNK(float, set_value_float_thunk, set_value);
    DEFINE_THUNK(int, set_value_int_thunk, set_value);
#undef DEFINE_THUNK

    // It doesnt look like IConVar::GetName and IConVar::IsFlagSet are called
    static auto __fastcall undefined_thunk(u8 *ecx, void *edx, int arg1) {
        assert(0);
    }

    virtual auto create_convar(char const *name, char const *default_value, u32 flags, char const *help_string,
                               bool has_min, float min, bool has_max, float max, ChangeCallbackFn change_callback) -> void {
        create_base(name, help_string, flags);

        {
            // Set up the MI vtables properly
            // IConvar.h
            /*
			virtual void SetValue( const char *pValue ) = 0;
			virtual void SetValue( float flValue ) = 0;
			virtual void SetValue( int nValue ) = 0;

			virtual const char *GetName( void ) const = 0;
			virtual bool IsFlagSet( int nFlag ) const = 0;
			*/

            // These all need to be properly thunked
            static auto iconvar_vtable = []() {
                auto ret = new void *[5];
                ret[0]   = &set_value_int_thunk;
                ret[1]   = &set_value_float_thunk;
                ret[2]   = &set_value_string_thunk;

                ret[3] = &undefined_thunk;
                ret[4] = &undefined_thunk;

                return ret;
            }();

            this->convar_vtable = iconvar_vtable;
        }
        parent = this;

        this->default_value = (default_value == nullptr) || (default_value[0] == '\0') ? "0.0" : default_value;

        this->has_min   = has_min;
        this->value_min = min;

        this->has_max   = has_max;
        this->value_max = max;

        this->change_callback = change_callback;

        set_value(this->default_value);
    }

public:
    ConCommandBase *next;

    bool registered;

    const char *name;
    const char *help_string;

    u32 flags;

    static ConCommandBase *head;

    // Convar is an mi class and therefore needs this
    void *convar_vtable;

    // Convar members
    ConCommandBase *parent;

    // Static data
    const char *default_value;

    // Value
    // Dynamically allocated
    char *value_string;
    int   value_string_length;

    // Values
    float value_float;
    int   value_int;

    // Min/Max values
    bool  has_min;
    float value_min;
    bool  has_max;
    float value_max;

    ChangeCallbackFn change_callback;
};

ConCommandBase *ConCommandBase::head;
} // namespace TF

const Convar_Base *Convar_Base::head = nullptr;

auto Convar_Base::tf_convar_changed(TF::IConVar *iconvar, const char *old_string, float old_float) -> void {
    auto convar = TF::ConCommandBase::from_iconvar(iconvar);
    assert(convar);

    for (auto c : Convar_Base::get_range()) {
        if (c->tf_convar == convar) {
            if (convar->registered == false) return;
            if (c->init_complete == false) return;

            auto modifiable  = const_cast<Convar_Base *>(c);
            auto was_clamped = modifiable->from_string(convar->value_string);

            // Remove the callback when we call set_value to prevent recursion
            // TODO: there is probably a better way to do this...
            auto callback_backup    = convar->change_callback;
            convar->change_callback = nullptr;
            if (was_clamped) convar->set_value(modifiable->to_string());
            convar->change_callback = callback_backup;

            Log::msg("Updated convar %s to '%s' (%s)", convar->get_name(), convar->value_string, was_clamped ? "clamped" : "not clamped");
        }
    }
}

Convar_Base::Convar_Base(const char *name, Convar_Type type, const Convar_Base *parent) : parent(parent), t(type), next(head), init_complete(false) {
    head = this;

    strcpy_s(internal_name, name);

    // Create a tf convar based on this one
    tf_convar = new TF::ConCommandBase;
    tf_convar->create_convar(name, "", 0, name, false, 0, false, 0, &Convar_Base::tf_convar_changed);

    init_complete = true;
}

Convar_Base::~Convar_Base() {

    if (this == head) head = this->next;

    for (auto c : Convar_Base::get_range()) {
        auto modifiable = const_cast<Convar_Base *>(c);
        if (c->next == this) modifiable->next = this->next;
    }
}

auto Convar_Base::init_all() -> void {
    assert(IFace<TF::Cvar>());

    // We have to do this goofy loop here as register_command() will
    // change the `next` pointer to the next in its chain
    // which will cause all kinds of problems for us

    // TODO: this could also be fixed by using the Convar_Base linked list...

    auto c = TF::ConCommandBase::head;
    while (c != nullptr) {
        auto next = c->next;

        IFace<TF::Cvar>()->register_command(c);

        c = next;
    }
}
