#pragma once

#include "math.hh"

#include <algorithm>

namespace TF {
class Entity;
}

namespace BlueTarget {
using Target = std::pair<TF::Entity *, Math::Vector>;
}
// This class is what calls out to the other module's createmoves
// Doing it this way allows us to keep code clean and seperated whilst
// still allowing for the same functionality

class CreateMoveModule {
public:
    static void update(float frametime) {
    }
    static void level_startup();
    static void level_shutdown();
};
