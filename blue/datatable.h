#pragma once

#include <cassert>

#include "types.h"

// TODO: should this be namespace TF?
namespace TF {
class RecvTable;
class RecvProp;
class RecvDecoder;

struct RecvProxyData {
public:
    const RecvProp *prop;      // The property it's receiving.
    u8              value[16]; // The value given to you to store.
    int             element;   // Which array element you're getting.
    int             id;        // The object being referred to.
};

class RecvProp {
    typedef void (*RecvVarProxyFn)(const RecvProxyData *data, void *this_ptr, void *out);
    typedef void (*ArrayLengthRecvProxyFn)(void *this_ptr, int id, int cur_length);
    typedef void (*DataTableRecvVarProxyFn)(const RecvProp *prop, void **out, void *data, int id);

public:
    RecvProp() = delete;

    auto get_num_elements() const {
        return element_count;
    }

    auto get_element_stride() const {
        return element_stride;
    }

    auto get_flags() const {
        return flags;
    }

    auto get_name() const {
        return network_name;
    }
    auto get_type() const {
        return recv_type;
    }

    auto get_proxy_fn() const {
        return proxy_fn;
    }
    auto set_proxy_fn(RecvVarProxyFn fn) {
        proxy_fn = fn;
    }

    auto get_datatable() const {
        return data_table;
    }

    auto get_datatable_proxy_fn() const {
        return datatable_proxy_fn;
    }
    auto set_datatable_proxy_fn(DataTableRecvVarProxyFn fn) {
        datatable_proxy_fn = fn;
    }

    auto get_offset() const {
        return offset;
    }

    auto get_array_prop() const {
        return array_prop;
    }
    auto get_array_length_proxy() const {
        return array_length_proxy;
    }
    void set_array_length_proxy(ArrayLengthRecvProxyFn proxy) {
        array_length_proxy = proxy;
    }
    auto is_inside_array() const {
        return inside_array;
    }

    auto get_extra_data() const {
        return extra_data;
    }

    // If it's one of the numbered "000", "001", etc properties in an array, then
    // these can be used to get its array property name for debugging.
    auto get_parent_name() {
        return parent_array_name;
    }

public:
    const char *network_name;
    int         recv_type;
    int         flags;
    int         string_buffer_size;

private:
    bool inside_array; // Set to true by the engine if this property sits inside an array.

    // Extra data that certain special property types bind to the property here.
    const void *extra_data;

    // If this is an array (DPT_Array).
    RecvProp *             array_prop;
    ArrayLengthRecvProxyFn array_length_proxy;

    RecvVarProxyFn          proxy_fn;
    DataTableRecvVarProxyFn datatable_proxy_fn; // For RDT_DataTable.

    RecvTable *data_table; // For RDT_DataTable.
    int        offset;

    int element_stride;
    int element_count;

    // If it's one of the numbered "000", "001", etc properties in an array, then
    // these can be used to get its array property name for debugging.
    const char *parent_array_name;
};

class RecvTable {
public:
    RecvTable() = delete;

    auto get_num_props() {
        return prop_count;
    }

    auto get_prop(int i) {
        assert(i < prop_count);
        return &props[i];
    }

    auto name() {
        return table_name;
    }

    // C++11 iterator functions
    auto begin() { return get_prop(0); }
    auto end() { return props + prop_count; }

public:
    // Properties described in a table.
    RecvProp *props;
    int       prop_count;

    // The decoder. NOTE: this covers each RecvTable AND all its children (ie: its children
    // will have their own decoders that include props for all their children).
    RecvDecoder *decoder;

    const char *table_name; // The name matched between client and server.
};
} // namespace TF