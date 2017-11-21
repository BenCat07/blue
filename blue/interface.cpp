#include "stdafx.h"

#include "interface.h"
#include "log.h"
#include "signature.h"

using instantiate_interface_fn = void *(*)();

class InterfaceReg {
public:
    InterfaceReg() = delete;

public:
    instantiate_interface_fn create_fn;
    const char *             name;

    InterfaceReg *next;
};

// TODO: have this resolve modules aswell / make platform independent
auto Interface_Helpers::find_interface(const char *module_name, const char *interface_name) -> void * {
    auto pattern = Signature::Pattern(module_name,
                                      "8B 35 ? ? ? ? 57 85 F6 74 38",
                                      "",
                                      "");
    // TODO: this is basically completely useless
    // as different compilers may implement this differently
    // so you may need more or less derefs
    auto interface_reg_head = **pattern.find<InterfaceReg ***>(2);
    assert(interface_reg_head);

    for (auto r = interface_reg_head; r != nullptr; r = r->next) {
        auto match = strstr(r->name, interface_name);
        if (match != nullptr && !isalpha(*(match + strlen(interface_name)))) return r->create_fn();
    }

    assert(0);

    return nullptr;
}
