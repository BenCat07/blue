#include "stdafx.hh"

#include "convar.hh"

const Convar_Base *Convar_Base::head = nullptr;

Convar_Base::Convar_Base(const char *name, Convar_Type type, const Convar_Base *parent) : parent(parent), t(type), next(head) {
    head = this;

    strcpy_s(internal_name, name);
}

Convar_Base::~Convar_Base() {

    if (this == head) head = this->next;

    for (auto c : Convar_Base::get_range()) {
        auto modifiable = const_cast<Convar_Base *>(c);
        if (c->next == this) modifiable->next = this->next;
    }
}
