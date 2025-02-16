#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class StateSystem {
public:
    void init();
    void step(float elapsed_ms);
private:
    RenderRequest* request = nullptr;
    State* state = nullptr;
};
