#pragma once
#include "blue_platform.hh"

#include "blue_list.hh"

namespace BlueModule {

// BLUE_CREATE_INVOKE is defined in blue_list.hh

template <typename Derived>
class Invoke {
public:
    BLUE_CREATE_INVOKE(create_move_pre_predict);
    BLUE_CREATE_INVOKE(create_move);
    BLUE_CREATE_INVOKE(level_shutdown);
    BLUE_CREATE_INVOKE(level_startup);
    BLUE_CREATE_INVOKE(update);
};

} // namespace BlueModule

START_LIST(ModuleList);

// include modules here
#include "aimbot.hh"
ADD_TO_LIST(ModuleList, Aimbot);
#include "module_createmove.hh"
ADD_TO_LIST(ModuleList, CreateMoveModule);

END_LIST(ModuleList);
