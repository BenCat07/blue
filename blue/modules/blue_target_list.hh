#pragma once

#include "blue_list.hh"

namespace TF {
class Entity;
}

namespace BlueTarget {

using Target = std::pair<TF::Entity *, Math::Vector>;
template <typename Derived>
class Invoke {
public:
    BLUE_CREATE_INVOKE(flush_targets);
    BLUE_CREATE_INVOKE(valid_target);
    BLUE_CREATE_INVOKE(visible_target);
    BLUE_CREATE_INVOKE(finished_target);
    BLUE_CREATE_INVOKE(sort_targets);
};
} // namespace BlueTarget

START_LIST(TargetList);

ADD_TO_LIST(TargetList, class Aimbot)

END_LIST(TargetList);
