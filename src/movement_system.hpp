#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class MovementSystem
{
    public:
    MovementSystem();
    ~MovementSystem();
    void step(float elapsed_ms);

    private:
    void checkBoundaries(float elapsed_ms);
};