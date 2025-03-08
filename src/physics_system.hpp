#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	PhysicsSystem();
	~PhysicsSystem();

	bool start_and_load_sounds();

	void step(float elapsed_ms);

	static bool collides(const Motion &motion1, const Motion &motion2);

private:
	void handle_projectile_collisions();
	void handle_arrows(float elapsed_ms);

	Mix_Chunk *injured_sound;
};