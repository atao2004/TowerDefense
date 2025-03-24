// In status_system.cpp
#include "status_system.hpp"
#include <iostream>
#include "world_system.hpp"

StatusSystem::StatusSystem()
{
    // Currently empty, but needed for the linker
}

StatusSystem::~StatusSystem()
{
    // Destroy music components
    if (player_death_sound != nullptr)
        Mix_FreeChunk(player_death_sound);
    Mix_CloseAudio();
}

void StatusSystem::step(float elapsed_ms)
{
        // Handle different types of status effects
        for (Entity entity : registry.statuses.entities)
        {
            update_zombie_attack(entity, elapsed_ms);
            handle_projectile_attack(entity, elapsed_ms);
        }

        // Clean up expired statuses after all processing
        for (auto entity : registry.statuses.entities)
        {
            remove_expired_statuses(entity);
        }

        handle_cooldowns(elapsed_ms);
        handle_hit_effects(elapsed_ms);
}

void StatusSystem::update_zombie_attack(Entity entity, float elapsed_ms)
{
    // First check if entity has both required components
    if (!registry.statuses.has(entity) || !registry.players.has(entity))
    {
        return;
    }

    // Only get the components after confirming they exist
    auto &status_comp = registry.statuses.get(entity);
    auto &player = registry.players.get(entity);

    for (auto &status : status_comp.active_statuses)
    {
        if (status.type == "attack")
        {
            player.health -= status.value;
            // std::cout << "Entity " << (int)entity << " took " << status.value
            //           << " attack damage. Health: " << player.health << std::endl;

            // If the creature is an entity, update the hp_percentage.
            if (registry.players.has(entity))
            {
                registry.screenStates.get(registry.screenStates.entities[0]).hp_percentage = player.health / PLAYER_HEALTH;
            }
        }
        if (registry.players.has(entity) && player.health <= 0)
        {
            Mix_PlayChannel(1, player_death_sound, 0);
            WorldSystem::game_over(); // You'll need to pass WorldSystem reference
            return;
        }

        if (registry.players.has(entity))
        {
            // Add hit effect
            registry.hitEffects.emplace_with_duplicates(entity);

            // Add screen shake
            auto &screen = registry.screenStates.get(registry.screenStates.entities[0]);
            screen.shake_duration_ms = 200.0f;
            screen.shake_intensity = 10.0f;
        }
    }
}

void StatusSystem::remove_expired_statuses(Entity entity)
{
    if (!registry.statuses.has(entity))
    {
        return;
    }

    auto &status_comp = registry.statuses.get(entity);
    auto initial_size = status_comp.active_statuses.size();

    // Use std::remove_if to remove expired statuses
    status_comp.active_statuses.erase(
        std::remove_if(
            status_comp.active_statuses.begin(),
            status_comp.active_statuses.end(),
            [](const Status &status)
            { return status.duration_ms <= 0; }),
        status_comp.active_statuses.end());
}

void StatusSystem::handle_cooldowns(float elapsed_ms)
{
    auto &registry_cooldown = registry.cooldowns;
    for (uint i = 0; i < registry_cooldown.size(); i++)
    {
        Cooldown &cooldown = registry_cooldown.components[i];
        cooldown.timer_ms -= elapsed_ms;
        if (cooldown.timer_ms <= 0)
        {
            Entity entity = registry_cooldown.entities[i];
            registry_cooldown.remove(entity);
        }
    }
}

void StatusSystem::handle_hit_effects(float elapsed_ms)
{
    for (Entity entity : registry.hitEffects.entities)
    {
        if (registry.hitEffects.has(entity))
        {
            auto &hit = registry.hitEffects.get(entity);

            // Update duration
            hit.duration_ms -= elapsed_ms;

            // Remove effect when done
            if (hit.duration_ms <= 0)
            {
                registry.hitEffects.remove(entity);
            }
        }
    }
}

bool StatusSystem::start_and_load_sounds()
{

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    player_death_sound = Mix_LoadWAV(audio_path("player_death_sound.wav").c_str());

    if (player_death_sound == nullptr)
    {
        fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
                audio_path("player_death_sound.wav").c_str());
        return false;
    }

    return true;
}

void StatusSystem::handle_projectile_attack(Entity entity, float elapsed_ms)
{
    // Check if entity has necessary components
    if (!registry.statuses.has(entity) || !registry.enemies.has(entity))
   
    {
        return;
    }

    auto &status_comp = registry.statuses.get(entity);
    auto &enemy = registry.enemies.get(entity);

    // Process each status
    for (auto it = status_comp.active_statuses.begin(); it != status_comp.active_statuses.end();)
   
    {
        if (it->type == "attack")
       
        {
            // Apply projectile damage
            enemy.health -= it->value;
            // std::cout << "Zombie hit by projectile for " << it->value
            //           << " damage. Health remaining: " << enemy.health << std::endl;

            // Remove attack status after applying damage
            it = status_comp.active_statuses.erase(it);

            // Only check for death after all damage is applied
            if (enemy.health <= 0 && !registry.deathAnimations.has(entity))
            {
                // Add death animation only if it doesn't already have one
                registry.deathAnimations.emplace(entity);

                // Increase player experience
                WorldSystem::increase_exp_plant();
            }
        }
        else
        {
            ++it;
        }
    }
}