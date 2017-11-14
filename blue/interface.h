#pragma once

namespace Interface_Helpers {
auto find_interface(const char *module_name, const char *interface_name) -> void *;
} // namespace Interface_Helpers

template <typename T>
class Interface {
    static T *value;

public:
    // interface name should be the name of the interface
    // without the version number
    // e.g. "VClient017" -> "VClient"
    static auto set_interface(const char *module_name, const char *interface_name) -> void {
        value = static_cast<T *>(
            Interface_Helpers::find_interface(module_name, interface_name));
    }

    auto operator-> () -> T *& {
        return value;
    }
};

template <typename T>
T *Interface<T>::value;

template <typename T>
using IFace = Interface<T>;