#include "death_system.hpp"
#include <iostream>

void DeathSystem::step(float elapsed_ms, WorldSystem& world_system)
{
	for (uint i = 0; i < registry.enemies.size(); i++) {
		Entity entity = registry.enemies.entities[i];
		Enemy& enemy = registry.enemies.components[i];
		// This is what you do when you kill a enemy.
		if (enemy.health <= 0 && !registry.deathAnimations.has(entity)) // check here added a guard
		{
			// Add death animation component
			DeathAnimation& death_anim = registry.deathAnimations.emplace(entity);
			death_anim.slide_direction = vec2(0, 0);
			death_anim.alpha = 1.0f;
			death_anim.duration_ms = 500.0f; // Animation lasts 0.5 seconds

			// Increase the counter that represents the number of zombies killed.
			world_system.increment_points();

			// Kung: Upon killing a enemy, increase the experience of the player or reset the experience bar when it becomes full.
			world_system.increase_exp_player();
		}
	}
}
