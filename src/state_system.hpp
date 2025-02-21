#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class StateSystem {
public:
    static void update_state(STATE state_new);
private:
    static void state_attack();
};
