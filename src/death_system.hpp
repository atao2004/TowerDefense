#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "world_system.hpp"

class DeathSystem {
public:
    void step(float elapsed_ms, WorldSystem& world_system);
};
