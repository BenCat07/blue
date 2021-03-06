#include <stdafx.hh>

#define PLACE_CHECKER

#include "class_id.hh"

#include "interface.hh"
#include "log.hh"
#include "sdk.hh"

using namespace TF;
using namespace ClassID;

InternalChecker::ClassIDChecker *InternalChecker::ClassIDChecker::head = nullptr;

InternalChecker::ClassIDChecker::ClassIDChecker(const char *name, const u32 value) : name(name), intended_value(value) {
    this->name           = name;
    this->intended_value = value;
    next                 = head;
    head                 = this;
}

static auto find_class_id(const char *name) {
    for (auto client_class = IFace<Client>()->get_all_classes();
         client_class != nullptr;
         client_class = client_class->next)
        if (strcmp(client_class->network_name, name) == 0) return client_class->class_id;

    return -1;
}

auto InternalChecker::ClassIDChecker::check_correct() -> bool {
    auto found_value = find_class_id(name);

    if (found_value == -1) {
        Log::msg("[ClassID] Unable to find correct value for '%s'", name);
        return false;
    }

    if (found_value != intended_value) {
        Log::msg("[ClassID] value for %s is wrong (wanted %d, got %d)", name, intended_value, found_value);
        return false;
    }

    return true;
}

auto InternalChecker::ClassIDChecker::check_all_correct() -> void {
    for (auto checker = InternalChecker::ClassIDChecker::head; checker != nullptr; checker = checker->next) {
        checker->check_correct();
    }
}
