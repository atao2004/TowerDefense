// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return {abs(motion.scale.x), abs(motion.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool PhysicsSystem::collides(const Motion &motion1, const Motion &motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp, dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	if (!registry.screenStates.get(registry.screenStates.entities[0]).game_over)
	{

		// Move each entity that has motion (players, and even towers [they have 0 for velocity])
		// based on how much time has passed, this is to (partially) avoid
		// having entities move at different speed based on the machine.
		auto &motion_registry = registry.motions;

		for (uint i = 0; i < motion_registry.size(); i++)
		{

			Motion &motion = motion_registry.components[i];
			Entity entity = motion_registry.entities[i];
			float step_seconds = elapsed_ms / 1000.f;
			motion.position.x += step_seconds * motion.velocity.x;
			motion.position.y += step_seconds * motion.velocity.y;
		}

		handle_projectile_collisions();
		// // check for collisions between all moving entities
		// ComponentContainer<Motion> &motion_container = registry.motions;
		// for(uint i = 0; i < motion_container.components.size(); i++)
		// {
		// 	Motion& motion_i = motion_container.components[i];
		// 	Entity entity_i = motion_container.entities[i];

		// 	// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		// 	for(uint j = i+1; j < motion_container.components.size(); j++)
		// 	{
		// 		Motion& motion_j = motion_container.components[j];
		// 		if (collides(motion_i, motion_j))
		// 		{
		// 			Entity entity_j = motion_container.entities[j];
		// 			// Create a collisions event
		// 			// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
		// 			// CK: why the duplication, except to allow searching by entity_id
		// 			registry.collisions.emplace_with_duplicates(entity_i, entity_j);
		// 			// registry.collisions.emplace_with_duplicates(entity_j, entity_i);
		// 		}
		// 	}
		// }
	}
}

void PhysicsSystem::handle_projectile_collisions()
{
	for (Entity projectile : registry.projectiles.entities)
	{
		Motion &proj_motion = registry.motions.get(projectile);
		Projectile &proj = registry.projectiles.get(projectile);

		// Check collision with zombies
		for (Entity zombie : registry.zombies.entities)
		{
			Motion &zombie_motion = registry.motions.get(zombie);

			if (collides(proj_motion, zombie_motion))
			{
				// Get or create status component
				StatusComponent *status_comp;
				if (registry.statuses.has(zombie))
				{
					status_comp = &registry.statuses.get(zombie);
				}
				else
				{
					status_comp = &registry.statuses.emplace(zombie);
				}

				// Add attack status
				Status attack_status{
					"attack",
					0.0f,
					proj.damage};
				status_comp->active_statuses.push_back(attack_status);

				// Add hit effect
				registry.hitEffects.emplace_with_duplicates(zombie);

				// Remove projectile
				if (!proj.invincible) {
					registry.remove_all_components_of(projectile);
				}
				break;
			}
		}
	}

	// Check projectiles is out of window bounds
	for (Entity projectile : registry.projectiles.entities)
	{
		if (!registry.motions.has(projectile))
		{
			continue;
		}

		Motion &motion = registry.motions.get(projectile);

		// Check if projectile is out of window bounds
		if (motion.position.x < 0 ||
			motion.position.x > WINDOW_WIDTH_PX ||
			motion.position.y < 0 ||
			motion.position.y > WINDOW_HEIGHT_PX)
		{
			// Remove projectile if it's out of bounds
			registry.remove_all_components_of(projectile);
			continue;
		}
	}
}