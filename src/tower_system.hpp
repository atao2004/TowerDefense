#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class TowerSystem
{
public:
    TowerSystem();
    ~TowerSystem();

    void step(float elapsed_ms);

private:
    // Helper functions
    bool find_nearest_enemy(Entity tower, Entity& target);
    void fire_projectile(Entity tower, Entity target);
    float compute_delta_distance(Entity tower, Entity target);
};