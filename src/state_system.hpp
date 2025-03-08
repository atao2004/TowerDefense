#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class StateSystem {
public:
    static void update_state(STATE state_new);
    static STATE get_state();
};
