#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class PlayerSystem {
public:
    static void update_state(STATE state_new);
    static STATE get_state();
};
