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
#include "state_system.hpp"
#include "animation_system.hpp"
#include "tower_system.hpp"
#include "movement_system.hpp"
#include "frame_manager.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;
	StatusSystem  status_system;
	AnimationSystem animation_system;
	TowerSystem tower_system;
	MovementSystem movement_system;
	FrameManager frame_manager;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds in world_system." << std::endl;
	}

	if (!ai_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds in ai_system." << std::endl;
	}

	if (!status_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds in status_system." << std::endl;
	}

	if (!physics_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds in status_system." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);
	animation_system.init(&renderer_system);

	// variable timestep loop
	auto t = Clock::now();

	float fps_sum = 0;
	int record_times = 0;
	int max_fps = 0;
	int min_fps = 50000; //impossible number technically, lazy implementation sorry!
	int cooldown = 1000;

	// frame intervals
	int world_interval = 1;
	int ai_interval = 1;
	int physics_interval = 1;
	int status_interval = 1;
	int tower_interval = 1;
	int movement_interval = 1;
	int animation_interval = 1;

	while (!world_system.is_over()) {

		GAME_SCREEN_ID game_screen = world_system.get_game_screen();
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		frame_manager.tick();

		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		//when level up, we want the screen to be frozen
		if (StateSystem::get_state() != STATE::LEVEL_UP) {
			if (frame_manager.can_update(world_interval)) world_system.step(elapsed_ms);
			if (!WorldSystem::game_is_over) {

				//M2: FPS
				float current_fps = (1/(elapsed_ms/1000));
				cooldown -= elapsed_ms;
				if (cooldown <= 0) {                             //used to prevent screen flickering
					// std::cout<<"FPS: "<<current_fps<<std::endl;
					std::stringstream title_ss;
					title_ss <<"Farmer Defense: The Last Days"
							<< " | LEVEL: "<< world_system.level 
							<<" | SEED COUNT: "<< registry.inventorys.components[0].seedCount[world_system.current_seed]
							<<"| FPS: " << (int)current_fps;
							
					glfwSetWindowTitle(window, title_ss.str().c_str());
					cooldown = 1000;
				}
				if (record_times > 2) {     //ignore the first 2, outliers wow.. maximum 5000 and minimum 10-ish fps, crazy
					max_fps = max_fps < current_fps ? current_fps: max_fps;
					min_fps = min_fps > current_fps ? current_fps: min_fps;
					fps_sum += (1/(elapsed_ms/1000));
				}
				record_times++;

				if (frame_manager.can_update(ai_interval)) ai_system.step(elapsed_ms);
				if (frame_manager.can_update(physics_interval)) physics_system.step(elapsed_ms);
				if (frame_manager.can_update(status_interval)) status_system.step(elapsed_ms);
				if (frame_manager.can_update(tower_interval)) tower_system.step(elapsed_ms);
				if (frame_manager.can_update(movement_interval)) movement_system.step(elapsed_ms, game_screen);
				if (frame_manager.can_update(animation_interval)) animation_system.step(elapsed_ms);
			} else {
				//M2: FPS. make sure we only print once, lazy implementation
				if (record_times != 0) {
					std::cout<<"maximum FPS: "<<max_fps<<std::endl;
					std::cout<<"minimum FPS: "<<min_fps<<std::endl;
					std::cout<<"average FPS: "<<(fps_sum/record_times-2)<<std::endl;
					max_fps = 50000; min_fps = 0; fps_sum = 0; record_times = 0; record_times = 0;
				}
			}
		}
		
		glm::mat4 trans = glm::mat4(1.0f);
		renderer_system.renderText("hello", 100, 100, 1, {1, 1, 0}, trans);

		renderer_system.draw(game_screen);
	}

	return EXIT_SUCCESS;
}
