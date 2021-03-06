#include "stdafx.hh"

#include "interface.hh"
#include "log.hh"
#include "signature.hh"

using InstantiateInterfaceFn = void *(*)();

class InterfaceReg {
public:
    InterfaceReg() = delete;

public:
    InstantiateInterfaceFn create_fn;
    const char *             name;

    InterfaceReg *next;
};

auto Interface_Helpers::find_interface(const char *module_name, const char *interface_name) -> void * {

    InterfaceReg *interface_reg_head;

    if constexpr (BluePlatform::windows()) {
        interface_reg_head = **Signature::find_pattern<InterfaceReg ***>(module_name, "8B 35 ? ? ? ? 57 85 F6 74 38", 2);
    } else if constexpr (BluePlatform::linux()) {
        static_assert(BluePlatform::linux() == false);
    } else if constexpr (BluePlatform::osx()) {
        static_assert(BluePlatform::linux() == false);
    }

    assert(interface_reg_head);

    for (auto r = interface_reg_head; r != nullptr; r = r->next) {
        auto match = strstr(r->name, interface_name);

        // Only match if the next character after the name is the number
        if (match != nullptr && !isalpha(*(match + strlen(interface_name)))) return r->create_fn();
    }

    // if you get here either
    // 1: your interface name is wrong
    // 2: your dll name is wrong
    assert(0);

    return nullptr;
}
