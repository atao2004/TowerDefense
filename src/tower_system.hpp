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
    void handle_tower_attacks(float elapsed_ms);
    bool find_nearest_enemy(vec2 tower_pos, float range, Entity& target);
    void fire_projectile(Entity tower, Entity target);
};