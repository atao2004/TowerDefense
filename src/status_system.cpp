// In status_system.cpp
#include "status_system.hpp"
#include <iostream>

StatusSystem::StatusSystem()
{
    // Currently empty, but needed for the linker
}

void StatusSystem::step(float elapsed_ms)
{
    // Handle different types of status effects
    for (Entity entity : registry.statuses.entities)
    {
        handle_attack(entity, elapsed_ms);
    }

    // Clean up expired statuses after all processing
    for (auto entity : registry.statuses.entities)
    {
        remove_expired_statuses(entity);
    }
}

void StatusSystem::handle_attack(Entity entity, float elapsed_ms)
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
            std::cout << "Entity " << (int)entity << " took " << status.value
                      << " attack damage. Health: " << player.health << std::endl;
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