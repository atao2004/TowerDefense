#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class StateSystem {
public:
    static void step(float elapsed_ms);
    static void update_state(STATE state_new);
private:
    static float timer_ms;
    static void state_idle();
    static void state_move();
};
