#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class StateSystem {
public:
    static void init();
    static void step(float elapsed_ms);
    static void update_state(STATE state_new);
private:
    static RenderRequest* request;
    static State* state;
};
