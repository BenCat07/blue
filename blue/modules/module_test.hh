#pragma once

#include "blue_module_list.hh"

class TestModule {
public:
    static void update(float frametime);

    static void level_startup();
    static void level_shutdown() {
    }
};

ADD_TO_LIST(ModuleList, TestModule);
