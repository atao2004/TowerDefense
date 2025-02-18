#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>


// Forward declare any needed classes
class RenderSystem;

class StatusSystem {
public:
    // Constructor with RenderSystem for visual effects if needed
    StatusSystem();
    ~StatusSystem();
    
    // Core loop update
    void step(float elapsed_ms);
    
    // Status handlers
    void handle_enemy_attack(Entity entity, float damage);

    bool start_and_load_sounds();

private:
    // Helper functions
    void remove_expired_statuses(Entity entity);

    // Cooldown
    void handle_cooldowns(float elapsed_ms);

    // Hit effects
    void handle_hit_effects(float elapsed_ms);

    Mix_Chunk *player_death_sound;
};