#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "world_system.hpp"

// Forward declare any needed classes
class RenderSystem;

class StatusSystem {
public:
    // Constructor with RenderSystem for visual effects if needed
    StatusSystem();
    ~StatusSystem();
    
    // Core loop update
    void step(float elapsed_ms, WorldSystem& world_system);
    
    // Status handlers
    void update_zombie_attack(Entity entity, float elapsed_ms, WorldSystem& world_system);

    bool start_and_load_sounds();

private:
    // Helper functions
    void remove_expired_statuses(Entity entity);

    // Cooldown
    void handle_cooldowns(float elapsed_ms);

    // Hit effects
    void handle_hit_effects(float elapsed_ms);
    
    // handle projectile attack
    void handle_projectile_attack(Entity entity, float elapsed_ms);

    Mix_Chunk *player_death_sound;
};