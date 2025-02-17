#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

// Forward declare any needed classes
class RenderSystem;

class StatusSystem {
public:
    // Constructor with RenderSystem for visual effects if needed
    StatusSystem();
    
    // Core loop update
    void step(float elapsed_ms);
    
    // Status handlers
    void handle_enemy_attack(Entity entity, float damage);

private:
    // Helper functions
    void remove_expired_statuses(Entity entity);

    // Cooldown
    void handle_cooldowns(float elapsed_ms);
};