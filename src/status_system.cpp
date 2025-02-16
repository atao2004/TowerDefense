// In status_system.cpp
#include "status_system.hpp"
#include <iostream>
#include "world_system.hpp"

StatusSystem::StatusSystem()
{
    // Currently empty, but needed for the linker
}

void StatusSystem::step(float elapsed_ms)
{
    // Handle different types of status effects
    for (Entity entity : registry.statuses.entities)
    {
        handle_enemy_attack(entity, elapsed_ms);
    }

    // Clean up expired statuses after all processing
    for (auto entity : registry.statuses.entities)
    {
        remove_expired_statuses(entity);
    }

    handle_cooldowns(elapsed_ms);
}

void StatusSystem::handle_enemy_attack(Entity entity, float elapsed_ms)
{
    // First check if entity has both required components
    if (!registry.statuses.has(entity) || !registry.creatures.has(entity))
    {
        return;
    }

    // Only get the components after confirming they exist
    auto &status_comp = registry.statuses.get(entity);
    auto &creature = registry.creatures.get(entity);


    for (auto &status : status_comp.active_statuses)
    {
        if (status.type == "attack")
        {
            creature.health -= status.value;
            std::cout << "Entity " << (int)entity << " took " << status.value
                      << " attack damage. Health: " << creature.health << std::endl;
        }
        if (registry.players.has(entity) && creature.health <= 0) {
            WorldSystem::game_over();  // You'll need to pass WorldSystem reference
            return;
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
    auto& registry_cooldown = registry.cooldowns;
    for (uint i = 0; i < registry_cooldown.size(); i++) {
        Cooldown& cooldown = registry_cooldown.components[i];
        cooldown.timer_ms -= elapsed_ms;
        if (cooldown.timer_ms <= 0) {
            Entity entity = registry_cooldown.entities[i];
            registry_cooldown.remove(entity);
        }
    }
}