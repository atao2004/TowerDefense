// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

PhysicsSystem::PhysicsSystem()
{
}

PhysicsSystem::~PhysicsSystem()
{
	// Destroy music components
	if (injured_sound != nullptr)
		Mix_FreeChunk(injured_sound);
	Mix_CloseAudio();
}

bool PhysicsSystem::start_and_load_sounds()
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

    injured_sound = Mix_LoadWAV(audio_path("injured_sound.wav").c_str());

    if (injured_sound == nullptr)
    {
        fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
                audio_path("injured_sound.wav").c_str());
        return false;
    }

    return true;
}


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

// Check if exactly one of the entities has a mesh
bool only_one_mesh(Entity a, Entity b, Mesh*& mesh_1, Motion*& motion_1, Motion*& motion_2)
{
	if (registry.meshPtrs.has(a) && !registry.meshPtrs.has(b)) {
		mesh_1 = registry.meshPtrs.get(a);
		motion_1 = &registry.motions.get(a);
		motion_2 = &registry.motions.get(b);
		return true;
	}
	if (!registry.meshPtrs.has(a) && registry.meshPtrs.has(b)) {
		mesh_1 = registry.meshPtrs.get(b);
		motion_1 = &registry.motions.get(b);
		motion_2 = &registry.motions.get(a);
		return true;
	}
	return false;
}

// Compute the cross product
float cross_product(vec2 a, vec2 b, vec2 c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// Perform a box-point collision test
bool collides_box_point(vec2& box_min, vec2& box_max, vec2& point)
{
	return (
		box_min.x <= point.x && point.x <= box_max.x &&
		box_min.y <= point.y && point.y <= box_max.y
		);
}

// Perform a triangle-point collision test
bool collides_triangle_point(std::vector<vec2>& triangle, vec2 point)
{
	float cross_1 = cross_product(triangle[0], triangle[1], point);
	float cross_2 = cross_product(triangle[1], triangle[2], point);
	float cross_3 = cross_product(triangle[2], triangle[0], point);
	return (
		(cross_1 > 0 && cross_2 > 0 && cross_3 > 0) ||
		(cross_1 < 0 && cross_2 < 0 && cross_3 < 0)
		);
}

// Perform a triangle-AABB collision test
bool collides_triangle_box(std::vector<vec2>& triangle, vec2& box_min, vec2& box_max)
{
	// check if the triangle is inside the box
	for (int i = 0; i < triangle.size(); i++) {
		if (collides_box_point(box_min, box_max, triangle[i])) return true;
	}

	// check if the box is inside the triangle
	if (collides_triangle_point(triangle, vec2(box_min.x, box_min.y))) return true;
	if (collides_triangle_point(triangle, vec2(box_min.x, box_max.y))) return true;
	if (collides_triangle_point(triangle, vec2(box_max.x, box_min.y))) return true;
	if (collides_triangle_point(triangle, vec2(box_max.x, box_max.y))) return true;

	return false;
}


// Perform a mesh-circle collision test (ignore other collisions)
bool collides_mesh(Entity a, Entity b)
{
	Mesh* mesh_1;
	Motion* motion_1;
	Motion* motion_2;
	if (only_one_mesh(a, b, mesh_1, motion_1, motion_2)) {
		vec2 box_min = vec2(motion_2->position.x - motion_2->scale.x / 2, motion_2->position.y - motion_2->scale.y / 2);
		vec2 box_max = vec2(motion_2->position.x + motion_2->scale.x / 2, motion_2->position.y + motion_2->scale.y / 2);
		for (int i = 0; i < mesh_1->vertex_indices.size(); i += 3) {
			std::vector<vec2> triangle;
			triangle.push_back(motion_1->position + motion_1->scale * vec2(mesh_1->vertices[mesh_1->vertex_indices[i]].position));
			triangle.push_back(motion_1->position + motion_1->scale * vec2(mesh_1->vertices[mesh_1->vertex_indices[i + 1]].position));
			triangle.push_back(motion_1->position + motion_1->scale * vec2(mesh_1->vertices[mesh_1->vertex_indices[i + 2]].position));
			if (collides_triangle_box(triangle, box_min, box_max)) return true;
		}
		return false;
	}
	else {
		return true;
	}
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
		handle_arrows(elapsed_ms);
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
		for (Entity enemy : registry.enemies.entities)
		{
			Motion &enemy_motion = registry.motions.get(enemy);

			if (collides(proj_motion, enemy_motion) && collides_mesh(projectile, enemy))
			{
				// Get or create status component
				StatusComponent *status_comp;
				if (registry.statuses.has(enemy))
				{
					status_comp = &registry.statuses.get(enemy);
				}
				else
				{
					status_comp = &registry.statuses.emplace(enemy);
				}

				// Add attack status
				Status attack_status{
					"attack",
					0.0f,
					proj.damage};
				status_comp->active_statuses.push_back(attack_status);

				// Add hit effect
				registry.hitEffects.emplace_with_duplicates(enemy);

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

void PhysicsSystem::handle_arrows(float elapsed_ms)
{
	for (auto entity : registry.arrows.entities)
	{
		Arrow &arrow = registry.arrows.get(entity);

		// Update lifetime
		arrow.lifetime_ms -= elapsed_ms;
		if (arrow.lifetime_ms <= 0)
		{
			registry.remove_all_components_of(entity);
			continue;
		}

		// Skip if no motion component
		if (!registry.motions.has(entity))
		{
			continue;
		}

		Motion &motion = registry.motions.get(entity);
		Entity source = arrow.source;

		// If arrow was fired by skeleton, check collision with player and towers
		if (registry.skeletons.has(source))
		{
			// Check collision with player
			for (auto player : registry.players.entities)
			{
				if (!registry.motions.has(player))
					continue;

				Motion &player_motion = registry.motions.get(player);
				if (collides(motion, player_motion))
				{
					// play the hit sound
					Mix_PlayChannel(2, injured_sound, 0);

					// Get or create status component for player
					StatusComponent *status_comp;
					if (registry.statuses.has(player))
					{
						status_comp = &registry.statuses.get(player);
					}
					else
					{
						status_comp = &registry.statuses.emplace(player);
					}

					// Add attack status to apply damage
					Status attack_status{
						"attack",
						0.0f,
						arrow.damage};
						status_comp->active_statuses.push_back(attack_status);
						

					// Add hit effect for visual feedback
					registry.hitEffects.emplace_with_duplicates(player);

					// Apply screen shake if screen state exists
					if (registry.screenStates.entities.size() > 0)
					{
						auto &screen = registry.screenStates.get(registry.screenStates.entities[0]);
						screen.shake_duration_ms = 200.0f;
						screen.shake_intensity = 5.0f;
					}

					// Remove arrow after hitting
					registry.remove_all_components_of(entity);
					break;
				}
			}

			// Check collision with towers
			for (auto tower : registry.towers.entities)
			{
				if (!registry.motions.has(tower))
					continue;

				Motion &tower_motion = registry.motions.get(tower);
				if (collides(motion, tower_motion))
				{
					// Deal damage to tower
					if (registry.towers.has(tower))
					{
						registry.towers.get(tower).health -= arrow.damage;

						// Add hit effect for visual feedback
						registry.hitEffects.emplace_with_duplicates(tower);

						// Check if tower is destroyed
						if (registry.towers.get(tower).health <= 0)
						{
							registry.remove_all_components_of(tower);
						}
					}

					// Remove arrow after hitting
					registry.remove_all_components_of(entity);
					break;
				}
			}
		}

		// Check if arrow is out of window bounds
		if (motion.position.x < 0 ||
			motion.position.x > WINDOW_WIDTH_PX ||
			motion.position.y < 0 ||
			motion.position.y > WINDOW_HEIGHT_PX)
		{
			// Remove arrow if it's out of bounds
			registry.remove_all_components_of(entity);
			continue;
		}
	}
}
