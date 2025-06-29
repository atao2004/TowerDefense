#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "animation_system.hpp"
#include "world_system.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

class AISystem
{
public:
	AISystem();
	~AISystem();

	void step(float elapsed_ms);

	bool start_and_load_sounds();

private:
	// Core movement and behavior functions
	void update_enemy_behaviors(float elapsed_ms);
	void update_zombie_movement(Entity entity, float elapsed_ms);
	void update_skeletons(float elapsed_ms);
	void update_orcriders(float elapsed_ms);

	// Movement calculation helpers
	vec2 calculate_direction_to_target(vec2 start_pos, vec2 target_pos);
	float calculate_distance_to_target(vec2 start_pos, vec2 target_pos);

	// Behavior-specific functions
	void handle_chase_behavior(Entity entity, float elapsed_ms);
	void update_enemy_melee_attack(Entity entity, float elapsed_ms);

	// State management (for future use)
	void update_enemy_state(Entity entity);

	Mix_Chunk *injured_sound;

	void update_squads(float elapsed_ms);
	void update_archer_circle_formation(Squad &squad, float elapsed_ms, Entity player);
	void update_orc_protection(Squad &squad, float elapsed_ms, Entity player);
	void update_knight_herding(Squad &squad, float elapsed_ms, Entity player);
};