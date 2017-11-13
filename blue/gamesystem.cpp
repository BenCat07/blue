#include "stdafx.h"

#include "gamesystem.h"
#include "log.h"
#include "signature.h"

static GameSystem *head = nullptr;
GameSystem::GameSystem() {
    next = head;
    head = this;
}

GameSystem::~GameSystem() {
    Log::msg("GameSystem::~GameSystem()");
}

auto GameSystem::add_all() -> void {
    auto add_pattern = Signature::pattern("client",
                                          "E8 ? ? ? ? 83 C4 04 8B 76 04 85 F6 75 D0",
                                          "E8 ? ? ? ? 8B 5B 04 85 DB 75 C1",
                                          "E8 ? ? ? ? 8B 7F 04 85 FF 75 A1");

    auto add_fn = Signature::resolve_callgate(add_pattern.find());
    assert(add_fn);

    for (auto system = head; system != nullptr; system = system->next) {
        // Call original Add() function
        ((void (*)(IGameSystem *))add_fn)(system);
        system->init();
    }

    // reset the list
    head = nullptr;
}
