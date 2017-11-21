#include "stdafx.h"

#include "interface.h"
#include "netvar.h"
#include "sdk.h"

using namespace TF;

static Netvar *head;

auto Netvar::Tree::populate_recursive(RecvTable *t, netvar_tree *nodes) -> void {
    for (auto i = 0; i < t->get_num_props(); i++) {
        auto *     prop     = t->get_prop(i);
        const auto new_node = new node();
        new_node->p         = prop;

        if (prop->get_type() == 6) populate_recursive(prop->get_datatable(), &new_node->children);

        nodes->emplace_back(std::make_pair(prop->get_name(), new_node));
    }
}

Netvar::Tree::Tree() {
}

auto Netvar::Tree::init() -> void {
    if (prop_tree.size() > 0) return;

    auto cc = IFace<Client>()->get_all_classes();
    while (cc != nullptr) {
        const auto new_node = new node();
        new_node->p         = nullptr;

        populate_recursive(cc->recv_table, &new_node->children);
        prop_tree.emplace_back(cc->recv_table->name(), new_node);

        cc = cc->next;
    }
}

auto Netvar::Tree::find_offset(std::vector<const char *> t) -> uptr {
    uptr total = 0;
    auto nodes = &prop_tree;

    for (auto &name : t) {

        auto end = nodes->end();
        for (auto it = nodes->begin(); it != end; ++it) {
            auto p = *it;

            if (strcmp(name, p.first) == 0) {
                nodes = &p.second->children;
                if (p.second->p != nullptr)
                    total += p.second->p->get_offset();
                break;
            }
        }
    }

    return total;
}

Netvar::Tree Netvar::netvar_tree;

auto Netvar::add_to_init_list() -> void {
    next = head;
    head = this;
}

auto Netvar::init() -> void {
    offset = netvar_tree.find_offset(tables);
}

auto Netvar::init_all() -> void {
    netvar_tree.init();

    for (auto &n = head; n != nullptr; n = n->next) {
        n->init();
    }
}
