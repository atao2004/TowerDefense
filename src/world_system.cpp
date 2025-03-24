// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <thread>
#include <string>

#include "physics_system.hpp"
#include "spawn_manager.hpp"
#include "state_system.hpp"
#include "../ext/json.hpp"
#include "../ext/nuklear.h"
// #include "../ext/nuklear_glfw_gl3.h"
using json = nlohmann::json;

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H
FT_Library library;

bool WorldSystem::game_is_over = false;
Mix_Chunk *WorldSystem::game_over_sound = nullptr;
GAME_SCREEN_ID WorldSystem::game_screen = GAME_SCREEN_ID::SPLASH;
int WorldSystem::current_day = 1;

// create the world
WorldSystem::WorldSystem() : points(0), level(1), current_seed(0)
{
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (night_bgm != nullptr)
		Mix_FreeMusic(night_bgm);
	if (day_bgm != nullptr)
		Mix_FreeMusic(day_bgm);
	if (combat_bgm != nullptr)
		Mix_FreeMusic(combat_bgm);
	if (sword_attack_sound != nullptr)
		Mix_FreeChunk(sword_attack_sound);
	if (running_on_grass_sound != nullptr)
		Mix_FreeChunk(running_on_grass_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	WINDOW_HEIGHT_PX = WINDOW_HEIGHT_PX * 3 / 4;
	WINDOW_WIDTH_PX = WINDOW_WIDTH_PX * 3 / 4;
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE); // GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Farmer Defense: The Last Days", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto mouse_button_pressed_redirect = [](GLFWwindow *wnd, int _button, int _action, int _mods)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds()
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

	night_bgm = Mix_LoadMUS(audio_path("night_bgm.wav").c_str());
	day_bgm = Mix_LoadMUS(audio_path("day_bgm.wav").c_str());
	combat_bgm = Mix_LoadMUS(audio_path("combat_bgm.wav").c_str());
	sword_attack_sound = Mix_LoadWAV(audio_path("sword_attack_sound.wav").c_str());
	running_on_grass_sound = Mix_LoadWAV(audio_path("running_on_grass.wav").c_str());
	WorldSystem::game_over_sound = Mix_LoadWAV(audio_path("game_over.wav").c_str());

	if (night_bgm == nullptr || day_bgm == nullptr || combat_bgm == nullptr || sword_attack_sound == nullptr || running_on_grass_sound == nullptr || game_over_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
				audio_path("night_bgm.wav").c_str(),
				audio_path("day_bgm.wav").c_str(),
				audio_path("combat_bgm.wav").c_str(),
				audio_path("sword_attack_sound.wav").c_str(),
				audio_path("running_on_grass.wav").c_str(),
				audio_path("game_over.mp3").c_str());
		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	// std::cout << "Starting music..." << std::endl;

	// Set all states to default
	restart_game();
	// restart_tutorial();
	// init_splash_screen();
	// createSplashScreen(renderer);
		glm::mat4 trans =  glm::ortho(0.0f, (float)WINDOW_WIDTH_PX, 0.0f, (float)WINDOW_HEIGHT_PX);
		renderer->renderText("hi", 10, 10, 1, {1, 0, 1}, trans);
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	for (Entity i : registry.seeds.entities)
	{

		if (!registry.moveWithCameras.has(i))
		{
			if (registry.seeds.get(i).timer <= 0)
			{
				vec2 pos;
				pos.x = registry.motions.get(i).position.x;
				pos.y = registry.motions.get(i).position.y;
				// std::cout << "x pos " << pos.x << " y pos " << pos.y << std::endl;
				registry.remove_all_components_of(i);
				registry.seeds.remove(i);
				createTower(renderer, {pos.x - GRID_CELL_WIDTH_PX / 2, pos.y - GRID_CELL_HEIGHT_PX / 2});
			}
			else
			{
				registry.seeds.get(i).timer -= elapsed_ms_since_last_update;
			}
		}
	}
	if (StateSystem::get_state() == STATE::LEVEL_UP)
	{
		registry.inventorys.components[0].seedCount[current_seed]++;
	}
	// Using the spawn manager to generate zombies
	if (WorldSystem::game_is_over)
	{
		assert(registry.screenStates.components.size() <= 1);
		ScreenState &screen = registry.screenStates.components[0];
		if (screen.game_over)
		{
			screen.lerp_timer += elapsed_ms_since_last_update;
		}
		if (screen.lerp_timer >= 10000)
		{
			screen.lerp_timer = 10000;
		}
	}

	if (registry.enemies.size() == 0 && current_bgm != night_bgm)
	{
		current_bgm = night_bgm;
		std::thread music_thread([this]()
								 {
			// Mix_FadeOutMusic(1000);
			Mix_HaltMusic();
			Mix_FadeInMusic(night_bgm, -1, 1000); });
		music_thread.detach();
	}
	else if (registry.enemies.size() > 0 && current_bgm != combat_bgm)
	{
		current_bgm = combat_bgm;
		std::thread music_thread([this]()
								 {
			// Mix_FadeOutMusic(1000);
			Mix_HaltMusic();
			Mix_FadeInMusic(combat_bgm, -1, 1000); });
		music_thread.detach();
	}

	if (!WorldSystem::game_is_over && game_screen != GAME_SCREEN_ID::SPLASH)
	{
		update_camera();
		// spawn_manager.step(elapsed_ms_since_last_update, renderer);
		updateDayInProgress(elapsed_ms_since_last_update);
		// Check and respawn tutorial enemies if needed
		if (game_screen == GAME_SCREEN_ID::TUTORIAL)
		{
			check_tutorial_enemies();
		}

		update_enemy_death_animations(elapsed_ms_since_last_update);
		update_movement_sound(elapsed_ms_since_last_update);
		update_screen_shake(elapsed_ms_since_last_update);

		// Summon the chicken when in low health
		ScreenState &screen = registry.screenStates.components[0];
		if (screen.hp_percentage < 0.25f && !chicken_summoned)
		{
			createChicken(renderer);
			chicken_summoned = true;
		}

		return true;
	}

	return true;
}

// Shared elements between restarting a game and a tutorial
void WorldSystem::restart_common_tasks()
{
	registry.clear_all_components();
	// for(Entity i: registry.seeds.entities) {
	// 	registry.seeds.remove(i);
	// }

	// Reset day counter and related variables
	current_day = 1;
	rest_timer_ms = 0.f;
	enemy_spawn_timer_ms = 0.f;
	enemies_spawned_today = 0;
	enemies_to_spawn_today = calculate_enemies_for_day(current_day);
	day_in_progress = true;

	chicken_summoned = false;

	current_bgm = night_bgm;
	// smooth fade in, thread to prevent blocking
	std::thread music_thread([this]()
							 { Mix_FadeInMusic(night_bgm, -1, 1000); });
	music_thread.detach(); // Let it run independently
	// set volume to 35%, max value is 128
	Mix_VolumeMusic(128 * 0.35);

	// Debugging for memory/component leaks
	registry.list_all_components();

	game_is_over = false;

	// Reset the spawn manager
	spawn_manager.reset();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	registry.screenStates.get(registry.screenStates.entities[0]).game_over = false;
	registry.screenStates.get(registry.screenStates.entities[0]).lerp_timer = 0.0;

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	int grid_line_width = GRID_LINE_WIDTH_PX;

	// Kung: Create the grass texture and scorched earth texture for the background and reset the pre-existing surfaces
	removeSurfaces();
	// commented out Kung's code
	for (int x = -SCORCHED_EARTH_DIMENSION_PX * 4; x < MAP_WIDTH_PX + SCORCHED_EARTH_DIMENSION_PX * 4; x += SCORCHED_EARTH_DIMENSION_PX)
	{
		for (int y = -SCORCHED_EARTH_DIMENSION_PX * 2; y < MAP_HEIGHT_PX + SCORCHED_EARTH_DIMENSION_PX * 2; y += SCORCHED_EARTH_DIMENSION_PX)
		{
			createScorchedEarth(vec2(x, y));
		}
	}

	// if the screenState exists, reset the health bar percentages
	if (registry.screenStates.size() != 0)
	{
		registry.screenStates.get(registry.screenStates.entities[0]).hp_percentage = 1.0;
		registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
	}
}

// More shared elements between restarting a game and a tutorial, this time involving the player and associated rendering
void WorldSystem::restart_overlay_renders(vec2 player_pos)
{
	// reset player and spawn player in the middle of the screen
	Entity player = createPlayer(renderer, player_pos);

	// reset camera position
	createCamera(renderer, player_pos);

	// Kung: Create the pause button and toolbar, and have them overlay the player
	// registry.pauses.clear();
	registry.toolbars.clear();
	// createPause();
	createToolbar(vec2(player_pos.x, player_pos.y + CAMERA_VIEW_HEIGHT * 0.45));
	createSeedInventory(vec2(player_pos.x - 191, player_pos.y + CAMERA_VIEW_HEIGHT * 0.45), registry.motions.get(player).velocity, current_seed);

	// Kung: Reset player movement so that the player remains still when no keys are pressed

	// Move left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities)
		{
			if (registry.motions.has(mwc_entity))
				registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
		}
	}

	// Move right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities)
		{
			if (registry.motions.has(mwc_entity))
				registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
		}
	}

	// Move down
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities)
		{
			if (registry.motions.has(mwc_entity))
				registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
		}
	}

	// Move up
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities)
		{
			if (registry.motions.has(mwc_entity))
				registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	std::cout << "Restarting game..." << std::endl;

	restart_common_tasks();

	// Set the level to level 1 and the game_screen to PLAYING.
	level = 1;
	game_screen = GAME_SCREEN_ID::PLAYING;

	// Kung: This is for Milestone #2. This creates the farmland.
	parseMap(false);

	// create grid lines and clear any pre-existing grid lines
	// Kung: I cleared the grid lines so that they would now render on top of my textures
	// vertical lines
	// for (int col = 0; col <= WINDOW_WIDTH_PX / GRID_CELL_WIDTH_PX; col++)
	// {
	// 	// width of 2 to make the grid easier to see
	// 	grid_lines.push_back(createGridLine(vec2(col * GRID_CELL_WIDTH_PX, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
	// }

	// // horizontal lines
	// for (int row = 0; row <= WINDOW_HEIGHT_PX / GRID_CELL_HEIGHT_PX; row++)
	// {
	// 	// width of 2 to make the grid easier to see
	// 	grid_lines.push_back(createGridLine(vec2(0, row * GRID_CELL_HEIGHT_PX), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
	// }

	restart_overlay_renders(vec2{CAMERA_VIEW_WIDTH / 2, CAMERA_VIEW_HEIGHT / 2});

	// start the spawn manager
	spawn_manager.start_game();

	// Print the starting level (Level 1)
	std::cout << "==== LEVEL " << level << " ====" << std::endl;
}

// Reset the world state to the tutorial mode state
void WorldSystem::restart_tutorial()
{
	std::cout << "Restarting tutorial..." << std::endl;

	restart_common_tasks();

	// Set the level to level 0 (non-existent) and the game_screen to TUTORIAL.
	level = 0;
	game_screen = GAME_SCREEN_ID::TUTORIAL;

	// Kung: This is for Milestone #2. This creates the farmland.
	parseMap(true);

	// create grid lines and clear any pre-existing grid lines
	// Kung: I cleared the grid lines so that they would now render on top of my textures
	// vertical lines
	// for (int col = 0; col <= WINDOW_WIDTH_PX / GRID_CELL_WIDTH_PX; col++)
	// {
	// 	// width of 2 to make the grid easier to see
	// 	grid_lines.push_back(createGridLine(vec2(col * GRID_CELL_WIDTH_PX, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
	// }

	// // horizontal lines
	// for (int row = 0; row <= WINDOW_HEIGHT_PX / GRID_CELL_HEIGHT_PX; row++)
	// {
	// 	// width of 2 to make the grid easier to see
	// 	grid_lines.push_back(createGridLine(vec2(0, row * GRID_CELL_HEIGHT_PX), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
	// }

	// create the tutorial assets
	createTutorialMove(vec2(TUTORIAL_WIDTH_PX * 0.1, TUTORIAL_HEIGHT_PX * -0.5));
	createTutorialAttack(vec2(TUTORIAL_WIDTH_PX * 0.35, TUTORIAL_HEIGHT_PX * -0.5));
	createTutorialPlant(vec2(TUTORIAL_WIDTH_PX * 0.6, TUTORIAL_HEIGHT_PX * -0.5));
	createTutorialRestart(vec2(TUTORIAL_WIDTH_PX * 0.85, TUTORIAL_HEIGHT_PX * -0.5));

	// create the arrows for the tutorial
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 4 - 15, TUTORIAL_HEIGHT_PX * 0.4));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 2 - 15, TUTORIAL_HEIGHT_PX * 0.4));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX * 0.75 - 15, TUTORIAL_HEIGHT_PX * 0.4));
	create_tutorial_enemies();
	restart_overlay_renders(vec2{TUTORIAL_WIDTH_PX * 0.05, TUTORIAL_HEIGHT_PX * 0.4});

	// Print the starting level (Level 0)
	std::cout << "==== LEVEL " << level << " ====" << std::endl;
}

// Create tutorial enemies at specific locations that respawn when killed
void WorldSystem::create_tutorial_enemies()
{
	// Create a zombie under the "Attack" tutorial board
	vec2 zombie_pos = vec2(TUTORIAL_WIDTH_PX * 0.4, TUTORIAL_HEIGHT_PX * 0.4);
	createZombie(renderer, zombie_pos);

	// Create a skeleton under the "Plant" tutorial board
	vec2 skeleton_pos = vec2(TUTORIAL_WIDTH_PX * 0.65, TUTORIAL_HEIGHT_PX * 0.4);
	createSkeleton(renderer, skeleton_pos);

	// std::cout << "Tutorial enemies created" << std::endl;
}

// Check if tutorial enemies need to be respawned
void WorldSystem::check_tutorial_enemies()
{
	bool zombie_exists = false;
	bool skeleton_exists = false;

	// Check if tutorial zombie exists
	for (Entity zombie : registry.zombies.entities)
	{
		if (registry.motions.has(zombie))
		{
			Motion &motion = registry.motions.get(zombie);

			zombie_exists = true;
			// Keep zombie in place by setting velocity to zero
			motion.velocity = vec2(0.0f, 0.0f);
		}
	}

	// Check if tutorial skeleton exists
	for (Entity entity : registry.skeletons.entities)
	{
		if (registry.motions.has(entity))
		{
			Motion &motion = registry.motions.get(entity);

			skeleton_exists = true;
			// Keep skeleton in place by setting velocity to zero
			motion.velocity = vec2(0.0f, 0.0f);
		}
	}
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// Helper function to handle what happens when the player does a mouse click
void WorldSystem::player_attack()
{
	Entity player = registry.players.entities[0];
	if (!registry.cooldowns.has(player) && StateSystem::get_state() != STATE::ATTACK)
	{
		// Play the sword attack sound
		Mix_PlayChannel(3, sword_attack_sound, 0);

		Motion less_f_ugly = registry.motions.get(registry.players.entities[0]);
		if (less_f_ugly.scale.x < 0)
		{ // face left = minus the range from position
			less_f_ugly.position.x -= registry.attacks.get(registry.players.entities[0]).range;
		}
		else
		{ // face right = add the range from position
			less_f_ugly.position.x += registry.attacks.get(registry.players.entities[0]).range;
		}
		Motion weapon_motion = Motion();
		weapon_motion.position = less_f_ugly.position;
		weapon_motion.angle = less_f_ugly.angle;
		weapon_motion.velocity = less_f_ugly.velocity;
		weapon_motion.scale = less_f_ugly.scale;

		for (int i = 0; i < registry.enemies.size(); i++)
		{
			if (PhysicsSystem::collides(weapon_motion, registry.motions.get(registry.enemies.entities[i])) // if enemy and weapon collide, decrease enemy health
				|| PhysicsSystem::collides(registry.motions.get(registry.players.entities[0]), registry.motions.get(registry.enemies.entities[i])))
			{
				Entity enemy = registry.enemies.entities[i];
				if (registry.enemies.has(enemy))
				{
					auto &enemy_comp = registry.enemies.get(enemy);
					enemy_comp.health -= registry.attacks.get(registry.players.entities[0]).damage;
					// std::cout << "wow u r attacking so nice cool cool" << std::endl;

					// Calculate knockback direction (from player to enemy)
					Motion &enemy_motion = registry.motions.get(enemy);
					Motion &player_motion = registry.motions.get(player);
					vec2 direction = enemy_motion.position - player_motion.position;
					float length = sqrt(dot(direction, direction));
					if (length > 0)
					{
						direction = direction / length; // Normalize
					}

					// Apply knockback velocity immediately
					float knockback_force = 1000.0f;
					enemy_motion.velocity += direction * knockback_force;

					// Add hit effect
					HitEffect &hit = registry.hitEffects.emplace_with_duplicates(enemy);

					// This is what you do when you kill a enemy.
					if (enemy_comp.health <= 0)
					{
						// Add death animation before removing
						Entity player = registry.players.entities[0];
						Motion &player_motion = registry.motions.get(player);
						vec2 slide_direction = {player_motion.scale.x > 0 ? 1.0f : -1.0f, 0.0f};

						// Add death animation component
						DeathAnimation &death_anim = registry.deathAnimations.emplace(enemy);
						death_anim.slide_direction = slide_direction;
						death_anim.alpha = 1.0f;
						death_anim.duration_ms = 500.0f; // Animation lasts 0.5 seconds

						// Increase the counter that represents the number of zombies killed.
						points++;
						// std::cout << "enemies killed: " << points << std::endl;

						// Kung: Upon killing a enemy, increase the experience of the player or reset the experience bar when it becomes full.
						if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage < 1.0)
						{
							registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += registry.attacks.get(registry.players.entities[0]).damage / PLAYER_HEALTH;
						} // Kung: If the bar is full, reset the player experience bar and upgrade the user level.
						else if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage >= 1.0)
						{
							// StateSystem::update_state(STATE::LEVEL_UP);
							// come back later!
							if (registry.inventorys.components[0].seedCount[current_seed] == 0)
							{
								createSeedInventory(vec2(player_motion.position.x - 191, player_motion.position.y + CAMERA_VIEW_HEIGHT * 0.45), player_motion.velocity, current_seed);
							}
							registry.inventorys.components[0].seedCount[current_seed]++; // increment the seed count
							registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
							level++;
							std::cout << "==== LEVEL " << level << " ====" << std::endl;
						}
					}
				}
			}
		}
		// Player State
		StateSystem::update_state(STATE::ATTACK);

		// Cooldown
		if (registry.cooldowns.has(player))
		{
			registry.cooldowns.remove(player);
		}
		Cooldown &cooldown = registry.cooldowns.emplace(player);
		cooldown.timer_ms = COOLDOWN_PLAYER_ATTACK;
	}
}

void WorldSystem::update_enemy_death_animations(float elapsed_ms)
{
	// Process each entity with a death animation
	for (Entity entity : registry.deathAnimations.entities)
	{
		auto &death_anim = registry.deathAnimations.get(entity);

		// Update alpha
		death_anim.duration_ms -= elapsed_ms;
		death_anim.alpha = death_anim.duration_ms / 500.0f; // Linear fade out

		// Update position with increased slide speed and distance
		if (registry.motions.has(entity))
		{
			auto &motion = registry.motions.get(entity);
			float slide_speed = 300.0f; // pixels per second

			// Calculate movement
			float step_seconds = elapsed_ms / 1000.0f;
			vec2 movement = death_anim.slide_direction * (slide_speed * step_seconds);

			// Apply movement
			motion.position += movement;
		}

		// Remove entity when animation is complete
		if (death_anim.duration_ms <= 0)
		{
			registry.remove_all_components_of(entity);

			// Kung: Upon killing a enemy, update the enemy count and print it to the console.
			// std::cout << "Enemy count: " << registry.zombies.size() << " zombies" << std::endl;
		}
	}
}

void WorldSystem::update_screen_shake(float elapsed_ms)
{
	auto &screen = registry.screenStates.get(registry.screenStates.entities[0]);
	if (screen.shake_duration_ms > 0)
	{
		screen.shake_duration_ms -= elapsed_ms;

		// Calculate random shake offset
		float intensity = screen.shake_intensity * (screen.shake_duration_ms / 200.0f);
		screen.shake_offset = {
			((rand() % 100) / 50.0f - 1.0f) * intensity,
			((rand() % 100) / 50.0f - 1.0f) * intensity};

		if (screen.shake_duration_ms <= 0)
		{
			screen.shake_offset = {0.f, 0.f};
		}
	}
}

// float runningSoundTimer = 0.0;

// Kung: To make the code easier to read, I split the player movement here.
// I was responsible for this but Ziqing implemented single and multi-button movement first.
// However, I then implemented boundary checking and the situation where opposing keys cause no movement.
// In addition, I did general debugging, including on Ziqing's initial code.
void WorldSystem::player_movement(int key, int action, Motion &player_motion)
{
	// Move left
	if (player_motion.position.x > PLAYER_LEFT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_LEFT_SPEED;
			}
		}
	}

	// Move right
	if (player_motion.position.x < PLAYER_RIGHT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_RIGHT_SPEED;
			}
		}
	}

	// Move down
	if (player_motion.position.y < PLAYER_DOWN_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_DOWN_SPEED;
			}
		}
	}

	// Move up
	if (player_motion.position.y > PLAYER_UP_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_UP_SPEED;
			}
		}
	}
}

// Version of player_movement that works for the tutorial mode.
void WorldSystem::player_movement_tutorial(int key, int action, Motion &player_motion)
{
	// Move left
	if (player_motion.position.x > PLAYER_LEFT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_LEFT_SPEED;
			}
		}
	}

	// Move right
	if (player_motion.position.x < PLAYER_RIGHT_BOUNDARY_TUTORIAL)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_RIGHT_SPEED;
			}
		}
	}

	// Move down
	if (player_motion.position.y < PLAYER_DOWN_BOUNDARY_TUTORIAL)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_DOWN_SPEED;
			}
		}
	}

	// Move up
	if (player_motion.position.y > PLAYER_UP_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
					registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_UP_SPEED;
			}
		}
	}
}

void WorldSystem::on_key(int key, int, int action, int mod)
{
	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		close_window();
		return;
	}

	// Resetting game with the 'R' button
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		// if (game_screen == GAME_SCREEN_ID::TUTORIAL)
		// {
		// 	restart_tutorial();
		// }
		// else
		// {
		restart_game();
		// }

		return;
	}

	//load
	if (action == GLFW_RELEASE && key == GLFW_KEY_MINUS)
	{
		loadGame();
		return;
	}

	//save 
	if (action == GLFW_RELEASE && key == GLFW_KEY_EQUAL)
	{
		saveGame();
		return;
	}

	// Debug
	if (action == GLFW_PRESS && key == GLFW_KEY_L)
	{
		registry.list_all_components();
	}

	if (game_screen == GAME_SCREEN_ID::SPLASH) {
		//implement
		return;
	}

	// when player is in the level up menu, disable some game inputs
	if (StateSystem::get_state() == STATE::LEVEL_UP ||
		game_is_over)
		return;	

	// Player movement
	Entity player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);

	// Manual wave generation with 'g'
	if (action == GLFW_PRESS && key == GLFW_KEY_G)
	{
		spawn_manager.generate_wave(renderer);
		return;
	}

	// test mode with '/'
	if (action == GLFW_PRESS && key == GLFW_KEY_SLASH)
	{
		// Disable in tutorial mode
		if (game_screen == GAME_SCREEN_ID::PLAYING || game_screen == GAME_SCREEN_ID::TEST)
		{
			test_mode = !test_mode;
			spawn_manager.set_test_mode(test_mode);
			if (game_screen == GAME_SCREEN_ID::PLAYING)
			{
				game_screen = GAME_SCREEN_ID::TEST;
			}
			else
				game_screen = GAME_SCREEN_ID::PLAYING;
			std::cout << "Game " << (test_mode ? "entered" : "exited") << " test mode" << std::endl;
		}
		return;
	}

	// tutorial mode with 't'
	if (action == GLFW_PRESS && key == GLFW_KEY_T)
	{
		tutorial_mode = !tutorial_mode;
		spawn_manager.set_test_mode(tutorial_mode);

		if (game_screen != GAME_SCREEN_ID::TUTORIAL)
		{
			restart_tutorial();
		}
		else
		{
			restart_game();
		}
		std::cout << "Game " << (tutorial_mode ? "entered" : "exited") << " tutorial mode" << std::endl;
		return;
	}

	// Calculate cell indices
	int cell_x = static_cast<int>(motion.position.x) / GRID_CELL_WIDTH_PX;
	int cell_y = static_cast<int>(motion.position.y) / GRID_CELL_HEIGHT_PX;

	// Kung: Plant seed with the 'F' button
	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		// You can only plant where there is farmland.
		for (Entity maptile_entity : registry.mapTiles.entities)
		{
			if (registry.motions.has(maptile_entity) && registry.renderRequests.has(maptile_entity))
			{
				if (registry.renderRequests.get(maptile_entity).used_texture == DECORATION_LIST[6])
				{
					if (registry.motions.get(maptile_entity).position == vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX))
					{
						// Remove any seeds that have already been planted to begin with.
						int hasSeed = 0;
						for (Entity motion_entity : registry.motions.entities)
						{
							if (registry.seeds.has(motion_entity) || registry.towers.has(motion_entity))
							{
								if (registry.motions.get(motion_entity).position == vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX))
								{
									hasSeed = 1;
									// std::cout << "A seed was already planted here." << std::endl;
								}
							}
						}
						if (!hasSeed)
						{
							if (registry.inventorys.components[0].seedCount[current_seed] > 0)
							{
								Entity seed = createSeed(vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX), current_seed);
								registry.inventorys.components[0].seedCount[current_seed]--; // decrease the count of seed in inventory
								if (registry.inventorys.components[0].seedCount[current_seed] == 0)
								{
									// Remove the seed from the toolbar.
									for (Entity seed_entity : registry.seeds.entities)
									{
										if (registry.moveWithCameras.has(seed_entity))
										{
											if (registry.seeds.get(seed_entity).type == current_seed)
											{
												registry.remove_all_components_of(seed_entity);
											}
										}
									}
								}
							}
							else
							{
								std::cout << "No more inventory of seed type " << std::endl;
							}
						}

						std::cout << "inventory count of seed type " << current_seed << " is " << registry.inventorys.components[0].seedCount[current_seed] << std::endl;
					}
				}
			}
		}
	}

	// Kung: Helper function for player movement (see above for description)
	if (game_screen == GAME_SCREEN_ID::TUTORIAL)
	{
		player_movement_tutorial(key, action, motion);
	}
	else
	{
		player_movement(key, action, motion);
	}

	// Update state if player is moving
	if (key == GLFW_KEY_A || key == GLFW_KEY_D || key == GLFW_KEY_S || key == GLFW_KEY_W)
	{
		if (motion.velocity == vec2(0, 0))
		{
			StateSystem::update_state(STATE::IDLE);
		}
		else
		{
			StateSystem::update_state(STATE::MOVE);
		}
	}

	// Player movement sound
	if (!WorldSystem::game_is_over)
	{

		if ((action == GLFW_PRESS || action == GLFW_REPEAT) &&
			(key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D))
		{
			if (!is_movement_sound_playing && movement_sound_timer <= 0)
			{
				Mix_PlayChannel(0, running_on_grass_sound, 0);
				is_movement_sound_playing = true;
				movement_sound_timer = 1000.f;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			if (motion.velocity.x == 0 && motion.velocity.y == 0)
			{
				if (is_movement_sound_playing)
				{
					Mix_HaltChannel(0);
					is_movement_sound_playing = false;
					movement_sound_timer = 0.f;
				}
			}
		}
	}

	// Chicken
	if (action == GLFW_PRESS && key == GLFW_KEY_C)
		createChicken(renderer);

	// Debug
	if (action == GLFW_PRESS && key == GLFW_KEY_0)
	{
		if (registry.players.size() > 0)
		{
			createSkeleton(renderer, vec2(motion.position.x + CAMERA_VIEW_WIDTH / 2, motion.position.y));
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_1)
	{
		if (registry.screenStates.size() != 0)
		{
			if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage >= 1.0)
			{
				// StateSystem::update_state(STATE::LEVEL_UP);
				// come back later!
				if (registry.inventorys.components[0].seedCount[current_seed] == 0)
				{
					createSeedInventory(vec2(motion.position.x - 191, motion.position.y + CAMERA_VIEW_HEIGHT * 0.45), motion.velocity, current_seed);
				}
				registry.inventorys.components[0].seedCount[current_seed]++; // increment the seed count
				registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
				level++;
				std::cout << "==== LEVEL " << level << " ====" << std::endl;
			}
			else
			{
				registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += 0.1;
			}
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;

	if (game_screen == GAME_SCREEN_ID::SPLASH) {
		//implement
		return;
	}

	if (StateSystem::get_state() == STATE::LEVEL_UP ||
		game_is_over)
		return;

	// change player facing direction
	Entity player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);

	// face left
	if (mouse_pos_x < WINDOW_WIDTH_PX / 2 && motion.scale.x > 0)
	{
		motion.scale.x = -motion.scale.x;
	}

	// face right
	if (mouse_pos_x > WINDOW_WIDTH_PX / 2 && motion.scale.x < 0)
	{
		motion.scale.x = -motion.scale.x;
	}
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
	if (!WorldSystem::game_is_over && game_screen != GAME_SCREEN_ID::SPLASH)
	{
		// on button press
		if (action == GLFW_PRESS)
		{

			int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
			int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

			// std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
			// std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
		}

		if (action == GLFW_RELEASE && action == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (StateSystem::get_state() == STATE::LEVEL_UP)
				return;
			player_attack();
		}
	}
}

void WorldSystem::game_over()
{
	std::cout << "Game Over!" << std::endl;
	game_is_over = true;
	registry.screenStates.get(registry.screenStates.entities[0]).game_over = true;
	Mix_HaltMusic();
	Mix_PlayChannel(0, WorldSystem::game_over_sound, 0);
	createGameOver();
}

void WorldSystem::update_movement_sound(float elapsed_ms)
{
	// Update movement sound
	if (is_movement_sound_playing)
	{
		movement_sound_timer -= elapsed_ms;

		// If timer expired and player is still moving, restart sound
		if (movement_sound_timer <= 0 &&
			!registry.players.entities.empty())
		{
			Entity player = registry.players.entities[0];
			Motion &motion = registry.motions.get(player);

			if (motion.velocity.x != 0 || motion.velocity.y != 0)
			{
				Mix_PlayChannel(0, running_on_grass_sound, 0);
				movement_sound_timer = 1000.f;
			}
		}
	}
}

void WorldSystem::update_camera()
{
	// Update camera position to follow player
	// std::cout<<registry.players.size()<<" "<<registry.cameras.size()<<std::endl;
	if (!registry.players.entities.empty() && !registry.cameras.entities.empty())
	{
		Entity player = registry.players.entities[0];
		Entity camera = registry.cameras.entities[0];

		Motion &player_motion = registry.motions.get(player);
		Camera &cam = registry.cameras.get(camera);

		// move camera to player position
		cam.position = player_motion.position;
	}
}

void WorldSystem::advance_to_next_day()
{
	current_day++;
	std::cout << "===== ADVANCING TO DAY " << current_day << " =====" << std::endl;

	// Calculate number of enemies for the new day with a reasonable progression curve
	enemies_to_spawn_today = calculate_enemies_for_day(current_day);

	// Reset day stats
	enemies_spawned_today = 0;
	day_in_progress = true;
	enemy_spawn_timer_ms = 0.f;

	// Reset chicken summon flag for the new day
	chicken_summoned = false;

	// Show day number as text overlay
	// Could create a temporary text entity here to display day number
	std::cout << "Day " << current_day << " will have " << enemies_to_spawn_today << " enemies" << std::endl;
}

// Helper function to calculate number of enemies per day
int WorldSystem::calculate_enemies_for_day(int day)
{
	// Base enemies for day 1
	const int BASE_ENEMIES = 5;

	// Progressive difficulty scaling:
	// - Days 1-3: Linear increase (+2 enemies per day)
	// - Days 4-7: Slightly faster increase (+3 enemies per day)
	// - Days 8+: Challenging increase (+4 enemies per day)

	if (day <= 1)
		return BASE_ENEMIES;
	else if (day <= 3)
		return BASE_ENEMIES + (day - 1) * 2;
	else if (day <= 7)
		return BASE_ENEMIES + 4 + (day - 3) * 3;
	else
		return BASE_ENEMIES + 16 + (day - 7) * 4;
}

void WorldSystem::updateDayInProgress(float elapsed_ms_since_last_update)
{
	if (day_in_progress)
	{
		// We're still spawning enemies for current day
		if (enemies_spawned_today < enemies_to_spawn_today)
		{
			// Time to spawn next enemy?
			enemy_spawn_timer_ms += elapsed_ms_since_last_update;
			if (enemy_spawn_timer_ms >= 1000.f)
			{ // 1 enemy per second
				// Spawn a single enemy
				spawn_manager.spawn_enemy(renderer);
				enemies_spawned_today++;
				enemy_spawn_timer_ms = 0.f;
			}
		}
		else
		{
			// All enemies for this day have been spawned
			day_in_progress = false;
		}
	}
	else if (registry.enemies.size() == 0)
	{
		// All enemies are defeated, time for rest period
		rest_timer_ms += elapsed_ms_since_last_update;

		// Display rest time remaining
		if (rest_timer_ms < 10000.f)
		{ // 10 second rest
			// Optional: Display countdown text
			float remaining = (10000.f - rest_timer_ms) / 1000.f;
			std::cout << "Next day in: " << (int)remaining << " seconds\r" << std::flush;
		}
		else
		{
			std::cout << std::endl; // Clear the countdown line
			// Rest period is over, advance to next day
			rest_timer_ms = 0.f;
			advance_to_next_day();
		}
	}
}



void WorldSystem::loadGame() {
	registry.clear_all_components();

	json jsonFile;
	std::ifstream file(PROJECT_SOURCE_DIR + std::string("data/reload/game_0.json"));
	file>>jsonFile;
	game_is_over = jsonFile["game_is_over"];
	game_screen = jsonFile["game_screen"];
	current_day = jsonFile["current_day"];
	current_seed = jsonFile["current_seed"];
	level = jsonFile["level"];

	json ss_json = jsonFile["0"][0];
	ScreenState& ss = registry.screenStates.components[0];
	ss.darken_screen_factor = ss_json["darken_screen_factor"];
	ss.exp_percentage = ss_json["exp_percentage"];
	ss.game_over = ss_json["game_over"];
	ss.game_over_counter_ms = ss_json["game_over_counter_ms"];
	ss.game_over_darken = ss_json["game_over_darken"];
	ss.hp_percentage = ss_json["hp_percentage"];
	ss.lerp_timer = ss_json["lerp_timer"];
	ss.shake_duration_ms = ss_json["shake_duration_ms"];
	ss.shake_intensity = ss_json["shake_intensity"];
	ss.shake_offset = vec2(ss_json["shake_offset"][0], ss_json["shake_offset"][1]);

	json attack_arr = jsonFile["1"];
	for (long unsigned int i=0; i<attack_arr.size(); i++) {
		json attack_json = attack_arr[i];
		Entity e = Entity(attack_json["entity"]);
		Attack& attack = registry.attacks.emplace(e);
		attack.range = attack_json["range"];
		attack.damage = attack_json["damage"];
	}

	json motion_arr = jsonFile["2"];
	for (long unsigned int i=0; i<motion_arr.size(); i++) {
		json motion = motion_arr[i];
		Entity e = Entity(motion["entity"]);
		Motion& m = registry.motions.emplace(e);
		m.position = vec2(motion["position"][0], motion["position"][1]);
		m.angle =  motion["angle"];
		m.velocity = vec2(motion["velocity"][0], motion["velocity"][1]);
		m.scale =  vec2(motion["scale"][0], motion["scale"][1]);
	}

	json collisions_arr = jsonFile["3"];
	for (long unsigned int i=0; i<collisions_arr.size(); i++) {
		json collision = collisions_arr[i];
		Entity e = Entity(collision["entity"].get<int>());
		Entity other = Entity(collision["other"].get<int>());
		registry.collisions.emplace(e, other);
	}

	//didnt add meshPtrs, maybe add constraints when chicken summoned cannot save lol

	json dimension_arr = jsonFile["5"];
	for (long unsigned int i=0; i<dimension_arr.size(); i++) {
		json dimension_json = dimension_arr[i];
		Entity e = Entity(dimension_json["entity"]);
		Dimension& dimension = registry.dimensions.emplace(e);
		dimension.height = dimension_json["height"];
		dimension.width  = dimension_json["width"];
	}

	json renderRequests_arr = jsonFile["6"];
	for (long unsigned int i=0; i<renderRequests_arr.size(); i++) {
		json rr_json = renderRequests_arr[i];
		Entity e = Entity(rr_json["entity"]);
		RenderRequest& rr = registry.renderRequests.emplace(e);
		rr.used_texture = (TEXTURE_ASSET_ID)rr_json["used_texture"];
		rr.used_effect = (EFFECT_ASSET_ID)rr_json["used_effect"];
		rr.used_geometry = (GEOMETRY_BUFFER_ID)rr_json["used_geometry"];
	}

	json tower_arr = jsonFile["8"];
	for (long unsigned int i=0; i<tower_arr.size(); i++) {
		json tower_json = tower_arr[i];
		Entity e = Entity(tower_json["entity"]);
		Tower& tower = registry.towers.emplace(e);
		tower.health = tower_json["health"];
		tower.damage = tower_json["damage"];
		tower.range = tower_json["range"];
		tower.timer_ms = tower_json["timer_ms"];
		tower.state = tower_json["state"];
	}

	json zombie_arr = jsonFile["10"];
	for (long unsigned int i=0; i<zombie_arr.size(); i++) {
		json zombie_json = zombie_arr[i];
		Entity e = Entity(zombie_json["entity"]);
		Zombie& zombie = registry.zombies.emplace(e);
		zombie.health = zombie_json["health"];
	}

	json zombieSpawn_arr = jsonFile["11"];
	for (long unsigned int i=0; i<zombieSpawn_arr.size(); i++) {
		json zombieSpawn_json = zombieSpawn_arr[i];
		Entity e = Entity(zombieSpawn_json["entity"]);
		ZombieSpawn& zombieSpawn = registry.zombieSpawns.emplace(e);
	}

	json player_arr = jsonFile["12"];
	for (long unsigned int i=0; i<player_arr.size(); i++) {
		json player_json = player_arr[i];
		Entity e = Entity(player_json["entity"]);
		Player& player = registry.players.emplace(e);
		player.health = player_json["health"];
	}

	json sc_arr = jsonFile["13"];
	for (long unsigned int i=0; i<sc_arr.size(); i++) {
		json sc_json = sc_arr[i];
		Entity e = Entity(sc_json["entity"]);
		StatusComponent& sc = registry.statuses.emplace(e);
		for (const auto& s : sc_json["active_statuses"]) {
			Status status;
			status.type = s["type"];
			status.duration_ms = s["duration_ms"];
			status.value = s["value"];
			sc.active_statuses.push_back(status);
		}
	}

	json states_arr = jsonFile["14"];
	for (long unsigned int i=0; i<states_arr.size(); i++) {
		json state_json = states_arr[i];
		Entity e = Entity(state_json["entity"]);
		State& state = registry.states.emplace(e);
		state.state = (STATE)state_json["state"];
	}

	json animation_arr = jsonFile["15"];
	for (long unsigned int i=0; i<animation_arr.size(); i++) {
		json animation_json = animation_arr[i];
		Entity e = Entity(animation_json["entity"]);
		Animation& animation = registry.animations.emplace(e);
		animation.runtime_ms = animation_json["runtime_ms"];
		animation.timer_ms = animation_json["timer_ms"];
		animation.pose = animation_json["pose"];
		animation.transition_ms = animation_json["transition_ms"];
		animation.pose_count = animation_json["pose_count"];
		animation.loop = animation_json["loop"];
		animation.lock = animation_json["lock"];
		animation.destroy = animation_json["destroy"];
		animation.textures = NULL;
	}
	
	json death_arr = jsonFile["16"];
	for (long unsigned int i=0; i<death_arr.size(); i++) {
		json death_json = death_arr[i];
		Entity e = Entity(death_json["entity"]);
		Death& death = registry.deaths.emplace(e);
	}

	json cooldown_arr = jsonFile["17"];
	for (long unsigned int i=0; i<cooldown_arr.size(); i++) {
		json cooldown_json = cooldown_arr[i];
		Entity e = Entity(cooldown_json["entity"]);
		Cooldown& cooldown = registry.cooldowns.emplace(e);
		cooldown.timer_ms = cooldown_json["timer_ms"];
	}

	json da_arr = jsonFile["18"];
	for (long unsigned int i=0; i<da_arr.size(); i++) {
		json da_json = da_arr[i];
		Entity e = Entity(da_json["entity"]);
		DeathAnimation& da = registry.deathAnimations.emplace(e);
		da.slide_direction = vec2(da_json["slide_direction"][0],da_json["slide_direction"][1]);
		da.alpha = da_json["alpha"];
		da.duration_ms = da_json["duration_ms"];
	}

	json he_arr = jsonFile["19"];
	for (long unsigned int i=0; i<he_arr.size(); i++) {
		json he_json = he_arr[i];
		Entity e = Entity(he_json["entity"]);
		HitEffect& he = registry.hitEffects.emplace(e);
		he.duration_ms = he_json["duration_ms"];
		he.is_white = he_json["is_white"];
	}

	json projectile_arr = jsonFile["20"];
	for (long unsigned int i=0; i<projectile_arr.size(); i++) {
		json projectile_json = projectile_arr[i];
		Entity e = Entity(projectile_json["entity"]);
		Entity source = Entity(projectile_json["source"]);
		Projectile& p = registry.projectiles.emplace(e);
		p.source = source;
		p.damage = projectile_json["damage"];
		p.speed = projectile_json["speed"];
		p.lifetime_ms = projectile_json["lifetime_ms"];
		p.direction = vec2(projectile_json["direction"][0], projectile_json["direction"][1]);
		p.invincible = projectile_json["invincible"];
	}

	json camera_arr = jsonFile["21"];
	for (long unsigned int i=0; i<camera_arr.size(); i++) {
		json camera_json = camera_arr[i];
		Entity e = Entity(camera_json["entity"]);
		Camera& camera = registry.cameras.emplace(e);
		camera.position = vec2(camera_json["position"][0], camera_json["position"][1]);
		camera.camera_width = camera_json["camera_width"];
		camera.camera_height = camera_json["camera_height"];
		camera.lerp_factor = camera_json["lerp_factor"];
	}

	json skeleton_arr = jsonFile["22"];
	for (long unsigned int i=0; i<skeleton_arr.size(); i++) {
		json skeleton_json = skeleton_arr[i];
		Entity e = Entity(skeleton_json["entity"]);
		Entity target = Entity(skeleton_json["target"]);
		Skeleton& skeleton = registry.skeletons.emplace(e);
		skeleton.attack_range = skeleton_json["attack_range"];
		skeleton.stop_distance = skeleton_json["stop_distance"];
		skeleton.attack_cooldown_ms = skeleton_json["attack_cooldown_ms"];
		skeleton.cooldown_timer_ms = skeleton_json["cooldown_timer_ms"];
		skeleton.target = target;
		skeleton.is_attacking = skeleton_json["is_attacking"];
		skeleton.health = skeleton_json["health"];
		skeleton.attack_timer_ms = skeleton_json["attack_timer_ms"];
		skeleton.arrow_fired = skeleton_json["arrow_fired"];
		skeleton.current_state = (Skeleton::State)skeleton_json["current_state"];
	}

	json arrow_arr = jsonFile["23"];
	for (long unsigned int i=0; i<arrow_arr.size(); i++) {
		json arrow_json = arrow_arr[i];
		Entity e = Entity(arrow_json["entity"]);
		Entity source = Entity(arrow_json["source"]);
		Arrow& a = registry.arrows.emplace(e);
		a.source = source;
		a.damage = arrow_json["damage"];
		a.speed = arrow_json["speed"];
		a.lifetime_ms = arrow_json["lifetime_ms"];
		a.direction = vec2(arrow_json["direction"][0], arrow_json["direction"][1]);
	}

	json visualScale_arr = jsonFile["24"];
	for (long unsigned int i=0; i<visualScale_arr.size(); i++) {
		json visualScale_json = visualScale_arr[i];
		Entity e = Entity(visualScale_json["entity"]);
		VisualScale& vs = registry.visualScales.emplace(e);
		vs.scale = vec2(visualScale_json["scale"][0], visualScale_json["scale"][1]);
	}

	json enemies_arr = jsonFile["25"];
	for (long unsigned int i=0; i<enemies_arr.size(); i++) {
		json enemies_json = enemies_arr[i];
		Entity e = Entity(enemies_json["entity"]);
		Enemy& enemy = registry.enemies.emplace(e);
		enemy.health = enemies_json["health"];
	}

	json inventory_arr = jsonFile["26"];
	for (long unsigned int i=0; i<inventory_arr.size(); i++) {
		json inventory_json = inventory_arr[i];
		Entity e = Entity(inventory_json["entity"]);
		Inventory& in = registry.inventorys.emplace(e);
		json seed_arr = inventory_json["seedCount"];
		for (long unsigned int i=0; i<seed_arr.size(); i++) {
			in.seedCount[i] = seed_arr[std::to_string(i)];
		}
	}

	json seed_arr = jsonFile["27"];
	for (long unsigned int i=0; i<seed_arr.size(); i++) {
		json seed_json = seed_arr[i];
		Entity e = Entity(seed_json["entity"]);
		Seed& seed = registry.seeds.emplace(e);
		seed.type = seed_json["type"];
		seed.timer = seed_json["timer"];
	}

	json mvc_arr = jsonFile["28"];
	for (long unsigned int i=0; i<mvc_arr.size(); i++) {
		json mvc_json = mvc_arr[i];
		Entity e = Entity(mvc_json["entity"]);
		registry.moveWithCameras.emplace(e);
	}

	std::cout<<"Game loaded successfully."<<std::endl;
}

void WorldSystem::saveGame() {
	json jsonFile;
	jsonFile["game_is_over"] = game_is_over;
	jsonFile["game_screen"] = game_screen;
	jsonFile["current_day"] = current_day;
	jsonFile["current_seed"] = current_seed;
	jsonFile["level"] = level;
	
	for (int i=0; i<registry.registry_list.size(); i++) {
		jsonFile[std::to_string(i)] = registry.registry_list[i]->toJSON();
	}
	

	std::ofstream outFile(PROJECT_SOURCE_DIR + std::string("data/reload/game_0.json"));
    if (outFile.is_open()) {
        outFile << jsonFile.dump(4);
        outFile.close();
        std::cout << "JSON written to file successfully.\n";
    } else {
        std::cerr << "Error opening file for writing.\n";
    }
}