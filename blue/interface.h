
namespace Interface_Helpers {
auto find_interface(const char *module_name, const char *interface_name) -> void *;
} // namespace Interface_Helpers

template <typename T>
class Interface : public T {
    Interface();

public:
    // interface name should be the name of the interface
    // without the version number
    // e.g. "VClient017" -> "VClient"
    static auto get_interface(const char *module_name, const char *interface_name) -> Interface<T> * {
        return static_cast<Interface<T> *>(
            Interface_Helpers::find_interface(module_name, interface_name));
    }
};