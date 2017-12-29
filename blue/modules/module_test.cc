#include <stdafx.hh>

#include "module_test.hh"

#include "log.hh"

void TestModule::update(float frametime) {
    //Log::msg("update %f", frametime);
}

void TestModule::level_startup() {
    Log::msg("wow i want to die");
}
