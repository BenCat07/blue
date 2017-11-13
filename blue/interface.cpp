#include "stdafx.h"

#include "interface.h"
#include "log.h"
#include "signature.h"

using instantiate_interface_fn = void *(*)();

class interface_reg {
public:
    interface_reg() = delete;

public:
    instantiate_interface_fn create_fn;
    const char *             name;

    interface_reg *next;
};

// TODO: have this resolve modules aswell / make platform independent
auto Interface_Helpers::find_interface(const char *module_name, const char *interface_name) -> void * {
    auto pattern = Signature::pattern("module_name",
                                      "8B 35 ? ? ? ? 57 85 F6 74 38",
                                      "",
                                      "");
    auto interface_reg_head = *pattern.find<interface_reg **>(2);
    assert(interface_reg_head);

    for (interface_reg *r = interface_reg_head; r != nullptr; r = r->next) {
        if (strstr(r->name, interface_name) != nullptr) return r->create_fn();
    }

    assert(0);

    return nullptr;
}
