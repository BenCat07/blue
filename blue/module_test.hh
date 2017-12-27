#pragma once

class TestModule {
public:
    static void update(float frametime) {
        Log::msg("time: %f", frametime);
    }
};

ADD_TO_LIST(ModuleList, TestModule);
