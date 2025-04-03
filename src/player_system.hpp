#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class PlayerSystem {
public:
    void step(float elapsed_ms);
    static void update_state(STATE state_new);
    static STATE get_state();
};
