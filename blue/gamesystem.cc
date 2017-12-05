#include "stdafx.hh"

#include "gamesystem.hh"
#include "log.hh"
#include "signature.hh"

static GameSystem *head = nullptr;
GameSystem::GameSystem() {
    next = head;
    head = this;
}

GameSystem::~GameSystem() {
    Log::msg("GameSystem::~GameSystem()");
}

auto GameSystem::add_all() -> void {
    using AddFn = void (*)(IGameSystem *);
    AddFn add_fn;

    if constexpr (BluePlatform::windows()) {
        add_fn = reinterpret_cast<AddFn>(
            Signature::resolve_callgate(
                Signature::find_pattern("client", "E8 ? ? ? ? 83 C4 04 8B 76 04 85 F6 75 D0")));
    } else if constexpr (BluePlatform::linux()) {
        add_fn = reinterpret_cast<AddFn>(
            Signature::resolve_callgate(
                Signature::find_pattern("client", "E8 ? ? ? ? 8B 5B 04 85 DB 75 C1")));
    } else if constexpr (BluePlatform::osx()) {
        add_fn = reinterpret_cast<AddFn>(
            Signature::resolve_callgate(
                Signature::find_pattern("client", "E8 ? ? ? ? 8B 7F 04 85 FF 75 A1")));
    }

    assert(add_fn);

    for (auto system = head; system != nullptr; system = system->next) {
        // Call original Add() function
        ((void (*)(IGameSystem *))add_fn)(system);
        system->init();
    }

    for (auto system = head; system != nullptr; system = system->next) {
        system->post_init();
    }

    // reset the list
    head = nullptr;
}
