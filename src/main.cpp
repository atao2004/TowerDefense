#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>
#include <sstream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "status_system.hpp"
#include "player_system.hpp"
#include "animation_system.hpp"
#include "tower_system.hpp"
#include "movement_system.hpp"
#include "screen_system.hpp"
#include "death_system.hpp"
// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include "particle_system.hpp"
#include "seed_system.hpp"
#include "frame_manager.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem ai_system;
	WorldSystem world_system;
	RenderSystem renderer_system;
	PhysicsSystem physics_system;
	StatusSystem status_system;
	AnimationSystem animation_system;
	TowerSystem tower_system;
	SeedSystem seed_system;
	MovementSystem movement_system;
	ParticleSystem particle_system;
	PlayerSystem player_system;
	ScreenSystem screen_system;
	DeathSystem death_system;

	// initialize window
	GLFWwindow *window = world_system.create_window();
	if (!window)
	{
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds())
	{
		std::cerr << "ERROR: Failed to start or load sounds in world_system." << std::endl;
	}

	if (!ai_system.start_and_load_sounds())
	{
		std::cerr << "ERROR: Failed to start or load sounds in ai_system." << std::endl;
	}

	if (!status_system.start_and_load_sounds())
	{
		std::cerr << "ERROR: Failed to start or load sounds in status_system." << std::endl;
	}

	if (!physics_system.start_and_load_sounds())
	{
		std::cerr << "ERROR: Failed to start or load sounds in status_system." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);
	animation_system.init(&renderer_system);
	particle_system.init(&renderer_system);
	seed_system.init(&renderer_system);

	// variable timestep loop
	auto t = Clock::now();

	float fps_sum = 0;
	int record_times = 0;
	int max_fps = 0;
	int min_fps = 50000; // impossible number technically, lazy implementation sorry!

	// frame intervals
	FrameManager fm_world = FrameManager(1);
	FrameManager fm_ai = FrameManager(1);
	FrameManager fm_physics = FrameManager(1);
	FrameManager fm_status = FrameManager(1);
	FrameManager fm_tower = FrameManager(5);
	FrameManager fm_movement = FrameManager(2);
	FrameManager fm_animation = FrameManager(2);
	FrameManager fm_particle = FrameManager(10);
	FrameManager fm_seed = FrameManager(5);
	FrameManager fm_render = FrameManager(5);
	FrameManager fm_player = FrameManager(5);
	FrameManager fm_screen = FrameManager(2);
	FrameManager fm_death = FrameManager(2);

	while (!world_system.is_over())
	{
		GAME_SCREEN_ID game_screen = world_system.get_game_screen();
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		//when level up, we want the screen to be frozen
		if (game_screen != GAME_SCREEN_ID::PAUSE && game_screen != GAME_SCREEN_ID::LEVEL_UP) {
			if (fm_world.can_update()) world_system.step(fm_world.get_time());
			if (!WorldSystem::game_is_over && game_screen != GAME_SCREEN_ID::SPLASH && game_screen != GAME_SCREEN_ID::CG ) {
				//M2: FPS
				FrameManager::tick(elapsed_ms); //moved here so when doing cg the game will pause
				float current_fps = (1/(elapsed_ms/1000));
				if (record_times > 2)
				{ // ignore the first 2, outliers wow.. maximum 5000 and minimum 10-ish fps, crazy
					max_fps = max_fps < current_fps ? current_fps : max_fps;
					min_fps = min_fps > current_fps ? current_fps : min_fps;
					fps_sum += (1 / (elapsed_ms / 1000));
				}
				record_times++;

				if (fm_ai.can_update())
					ai_system.step(fm_ai.get_time());
				if (fm_physics.can_update())
					physics_system.step(fm_physics.get_time());
				if (fm_status.can_update())
					status_system.step(fm_status.get_time(), world_system);
				if (fm_seed.can_update())
					seed_system.step(fm_seed.get_time());

				if (world_system.get_game_screen() == GAME_SCREEN_ID::CG)
					continue;

				if (fm_player.can_update())
					player_system.step(fm_player.get_time());
				if (fm_tower.can_update())
					tower_system.step(fm_tower.get_time());
				if (fm_movement.can_update())
					movement_system.step(fm_movement.get_time(), game_screen);
				if (fm_animation.can_update())
					animation_system.step(fm_animation.get_time());
				if (fm_particle.can_update())
					particle_system.step(fm_particle.get_time());
				if (fm_screen.can_update())
					screen_system.step(fm_screen.get_time());
				if (fm_death.can_update())
					death_system.step(fm_death.get_time(), world_system);
			}
			else
			{
				// M2: FPS. make sure we only print once, lazy implementation
				if (record_times != 0)
				{
					std::cout << "maximum FPS: " << max_fps << std::endl;
					std::cout << "minimum FPS: " << min_fps << std::endl;
					std::cout << "average FPS: " << (fps_sum / record_times - 2) << std::endl;
					max_fps = 50000;
					min_fps = 0;
					fps_sum = 0;
					record_times = 0;
					record_times = 0;
				}
			}
		}

		// DO NOT DELETE, OTHERWISE TEXT WON'T RENDER
		glm::mat4 trans = glm::mat4(1.0f);
		renderer_system.renderText("hello", 100, 100, 1, {1, 1, 0}, trans);
    
		if (fm_render.can_update() ||
		WorldSystem::game_is_over || game_screen == GAME_SCREEN_ID::CG || game_screen == GAME_SCREEN_ID::PAUSE || game_screen == GAME_SCREEN_ID::SPLASH || game_screen == GAME_SCREEN_ID::LEVEL_UP) 
			renderer_system.step_and_draw(fm_render.get_time());
    
	}
	return EXIT_SUCCESS;
}
