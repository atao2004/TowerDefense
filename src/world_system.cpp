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
#include "player_system.hpp"
#include "../ext/json.hpp"
using json = nlohmann::json;
#include "particle_system.hpp"

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library library;

bool WorldSystem::game_is_over = false;
Mix_Chunk *WorldSystem::game_over_sound = nullptr;
GAME_SCREEN_ID WorldSystem::game_screen = GAME_SCREEN_ID::SPLASH;
int WorldSystem::current_day = 1;
bool WorldSystem::player_is_dashing = false;

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
	OS_RES = OS_RES * 3 / 4;
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
	restart_splash_screen();
}

void WorldSystem::restart_splash_screen()
{
	game_screen = GAME_SCREEN_ID::SPLASH;
	createScreen(renderer, TEXTURE_ASSET_ID::BACKGROUND);
	createButton(renderer, BUTTON_ID::START, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5), vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5));
	createButton(renderer, BUTTON_ID::LOAD, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200), vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200));
	createButton(renderer, BUTTON_ID::TUTORIAL, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200 * 2), vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200 * 2));
	createButton(renderer, BUTTON_ID::QUIT, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200 * 3), vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 5 + 200 * 3));
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	if (registry.players.size() != 0 && registry.screenStates.size() != 0 && registry.inventorys.size() != 0) {
		increase_level();
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

	if (!WorldSystem::game_is_over && game_screen != GAME_SCREEN_ID::SPLASH && game_screen != GAME_SCREEN_ID::CG)
	{
		update_camera();
		// spawn_manager.step(elapsed_ms_since_last_update, renderer);
		if (game_screen == GAME_SCREEN_ID::PLAYING)
		{
			updateDayInProgress(elapsed_ms_since_last_update);
		}
		// Check and respawn tutorial enemies if needed
		if (game_screen == GAME_SCREEN_ID::TUTORIAL)
		{
			check_tutorial_enemies();
		}

		update_enemy_death_animations(elapsed_ms_since_last_update);
		update_movement_sound(elapsed_ms_since_last_update);
		update_screen_shake(elapsed_ms_since_last_update);

		update_dash(elapsed_ms_since_last_update);

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

void WorldSystem::print_level()
{
	registry.texts.clear();
	createText("Level: " + std::to_string(level), vec2(WINDOW_WIDTH_PX * 0.4, WINDOW_HEIGHT_PX - 75.0f), 0.75f, vec3(0.9f, 0.9f, 0.9f));
}

// Shared elements between restarting a game and a tutorial
void WorldSystem::restart_common_tasks(vec2 map_dimensions)
{
	registry.clear_all_components();

	registry.particles.clear();
    registry.particleGenerators.clear();
    registry.customData.clear();

	// Reset day counter and related variables
	current_day = 1; spawn_manager.set_day(current_day);
	rest_timer_ms = 0.f;
	enemy_spawn_timer_ms = 0.f;
	enemies_spawned_today = 0;
	enemies_to_spawn_today = calculate_enemies_for_day(current_day);
	day_in_progress = true;
	current_seed = 0;

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
	spawn_manager.squad_spawned = false;

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
	for (int x = -GRID_CELL_WIDTH_PX; x < map_dimensions.x + GRID_CELL_WIDTH_PX; x += GRID_CELL_WIDTH_PX)
	{
		for (int y = -GRID_CELL_HEIGHT_PX; y < map_dimensions.y + GRID_CELL_HEIGHT_PX; y += GRID_CELL_HEIGHT_PX)
		{
			if (x < 0 || y < 0)
			{
				createScorchedEarth(vec2(x, y));
			}
			else if (x >= map_dimensions.x || y >= map_dimensions.y)
			{
				createScorchedEarth(vec2(x, y));
			}
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
	Entity player = createPlayer(renderer, player_pos, current_seed);

	// reset camera position
	createCamera(renderer, player_pos);

	// Kung: Create the pause button and toolbar, and have them overlay the player
	registry.toolbars.clear();
	createPause(vec2(player_pos.x - CAMERA_VIEW_WIDTH / 2 + 30, player_pos.y - CAMERA_VIEW_HEIGHT / 2 + 30));
	createToolbar(vec2(player_pos.x, player_pos.y + CAMERA_VIEW_HEIGHT * 0.45));
	for(int i = 0; i < NUM_SEED_TYPES; i++) {
		if(registry.inventorys.components[0].seedCount[i] > 0) {
		createSeedInventory(vec2(player_pos.x - TOOLBAR_WIDTH / 2 + TOOLBAR_HEIGHT * (i * 0.995 + 0.5), player_pos.y + CAMERA_VIEW_HEIGHT * 0.45), registry.motions.get(player).velocity, i, i);	
		std::cout << "seed type: " << i << std::endl;
		}
	}
	// createSeedInventory(vec2(player_pos.x - TOOLBAR_WIDTH / 2 + TOOLBAR_HEIGHT * (current_seed + 0.5), player_pos.y + CAMERA_VIEW_HEIGHT * 0.45), registry.motions.get(player).velocity, current_seed);

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

void WorldSystem::start_cg(RenderSystem *renderer)
{
	registry.cgs.clear();
	game_screen = GAME_SCREEN_ID::CG;
	int cg_idx = registry.screenStates.components[0].cg_index;
	int cutscene = registry.screenStates.components[0].cutscene;
	if (cutscene == 1)
	{
		createScreen(renderer, TEXTURE_ASSET_ID::NIGHT_BG);
	}
	else
	{
		createScreen(renderer, TEXTURE_ASSET_ID::DAY_BG);
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	std::cout << "Restarting game..." << std::endl;

	restart_common_tasks(vec2(MAP_WIDTH_PX, MAP_HEIGHT_PX));

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
	print_level();
}

// Reset the world state to the tutorial mode state
void WorldSystem::restart_tutorial()
{
	std::cout << "Restarting tutorial..." << std::endl;

	restart_common_tasks(vec2(TUTORIAL_WIDTH_PX, TUTORIAL_HEIGHT_PX));

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
	createTutorialMove(vec2(TUTORIAL_WIDTH_PX * 0.1, TUTORIAL_SIGN_HEIGHT_PX));
	createTutorialAttack(vec2(TUTORIAL_WIDTH_PX * 0.35, TUTORIAL_SIGN_HEIGHT_PX));
	createTutorialPlant(vec2(TUTORIAL_WIDTH_PX * 0.6, TUTORIAL_SIGN_HEIGHT_PX));
	createTutorialRestart(vec2(TUTORIAL_WIDTH_PX * 0.85, TUTORIAL_SIGN_HEIGHT_PX));

	// create the arrows for the tutorial
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 4 - 15, TUTORIAL_ARROW_HEIGHT_PX));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 2 - 15, TUTORIAL_ARROW_HEIGHT_PX));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX * 0.75 - 15, TUTORIAL_ARROW_HEIGHT_PX));
	create_tutorial_enemies();
	restart_overlay_renders(vec2{TUTORIAL_WIDTH_PX * 0.05, TUTORIAL_ARROW_HEIGHT_PX});

	// Print the starting level (Level 0)
	print_level();
}

// Create tutorial enemies at specific locations that respawn when killed
void WorldSystem::create_tutorial_enemies()
{
	// Create a zombie under the "Attack" tutorial board
	vec2 zombie_pos = vec2(TUTORIAL_WIDTH_PX * 0.4, TUTORIAL_ARROW_HEIGHT_PX);
	createOrc(renderer, zombie_pos);

	// Create a skeleton under the "Plant" tutorial board
	vec2 skeleton_pos = vec2(TUTORIAL_WIDTH_PX * 0.65, TUTORIAL_ARROW_HEIGHT_PX);
	createSkeletonArcher(renderer, skeleton_pos);

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

// Helper function to increase the level automatically if the conditions are met.
void WorldSystem::increase_level() {
	Entity player_entity = registry.players.entities[0];
	// Kung: If the bar is full, reset the player experience bar and upgrade the user level.
	if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage >= 1.0) {
		if (registry.inventorys.components[0].seedCount[current_seed] == 0)
		{
			registry.inventorys.components[0].seedAtToolbar[current_seed] == -1;
			createSeedInventory(vec2(registry.motions.get(player_entity).position.x - TOOLBAR_WIDTH / 2 + TOOLBAR_HEIGHT * (current_seed + 0.5), registry.motions.get(player_entity).position.y + CAMERA_VIEW_HEIGHT * 0.45), registry.motions.get(player_entity).velocity, current_seed, 0);
		}
		registry.inventorys.components[0].seedCount[current_seed]++; // increment the seed count
		registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
		level++;

		vec2 player_pos = registry.motions.get(player_entity).position;
		vec2 player_size = registry.motions.get(player_entity).scale;
		ParticleSystem::createLevelUpEffect(player_pos, player_size);

		print_level();

		if (level == 2)
		{
			registry.screenStates.components[0].cutscene = 3;
			registry.screenStates.components[0].cg_index = 0;
			return start_cg(renderer);
		}
	}
}

// Helper function to make it easier to increase experience
void WorldSystem::increase_exp()
{
	Entity player_entity = registry.players.entities[0];
	if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage < 1.0)
	{
		registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += registry.attacks.get(player_entity).damage / PLAYER_HEALTH;
	}
}

void WorldSystem::increment_points()
{
	points++;
}

// Helper function to handle what happens when the player does a mouse click
void WorldSystem::player_attack()
{
	Entity player = registry.players.entities[0];
	if (!registry.cooldowns.has(player) && PlayerSystem::get_state() != STATE::ATTACK)
	{
		// Play the sword attack sound
		Mix_PlayChannel(3, sword_attack_sound, 0);

		Motion &player_motion = registry.motions.get(registry.players.entities[0]);

		// Calculate attack direction based on mouse position
		// Convert mouse position to world coordinates
		vec2 screen_center = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
		vec2 mouse_world_offset = vec2(mouse_pos_x - screen_center.x, mouse_pos_y - screen_center.y);

		// Normalize the direction vector
		vec2 attack_direction = normalize(mouse_world_offset);

		// Update player facing based on attack direction
		if (attack_direction.x != 0)
		{
			player_motion.scale.x = (attack_direction.x > 0) ? abs(player_motion.scale.x) : -abs(player_motion.scale.x);
		}

		// Calculate attack position based on direction and range
		vec2 attack_position = player_motion.position + attack_direction * static_cast<float>(registry.attacks.get(player).range);

		// Create weapon motion at the attack position
		Motion weapon_motion = Motion();
		weapon_motion.position = attack_position;
		weapon_motion.angle = atan2(attack_direction.y, attack_direction.x); // Set proper angle
		weapon_motion.velocity = player_motion.velocity;
		weapon_motion.scale = player_motion.scale;

		// Check for collisions with enemies
		for (int i = 0; i < registry.enemies.size(); i++)
		{
			if (PhysicsSystem::collides(weapon_motion, registry.motions.get(registry.enemies.entities[i])) // if enemy and weapon collide, decrease enemy health
				|| PhysicsSystem::collides(player_motion, registry.motions.get(registry.enemies.entities[i])))
			{
				Entity enemy = registry.enemies.entities[i];
				if (registry.enemies.has(enemy))
				{
					auto &enemy_comp = registry.enemies.get(enemy);
					enemy_comp.health -= registry.attacks.get(player).damage;

					// Calculate knockback direction (from player to enemy)
					Motion &enemy_motion = registry.motions.get(enemy);
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

					vec2 sprite_size = {50.0f, 50.0f}; // Default fallback size
					sprite_size = registry.motions.get(enemy).scale;
					// Create blood effect
					ParticleSystem::createBloodEffect(registry.motions.get(enemy).position, sprite_size);

					// This is what you do when you kill a enemy.
					if (enemy_comp.health <= 0 && !registry.deathAnimations.has(enemy))
					{
						// Add death animation before removing
						vec2 slide_direction = attack_direction; // Use attack direction for death animation

						// Add death animation component
						DeathAnimation &death_anim = registry.deathAnimations.emplace(enemy);
						death_anim.slide_direction = slide_direction;
						death_anim.alpha = 1.0f;
						death_anim.duration_ms = 500.0f; // Animation lasts 0.5 seconds

						// Increase the counter that represents the number of zombies killed.
						points++;

						// Kung: Upon killing a enemy, increase the experience of the player or reset the experience bar when it becomes full.
						increase_exp();
					}
				}
			}
		}

		// Player State
		PlayerSystem::update_state(STATE::ATTACK);

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
		restart_game();

		return;
	}

	// load
	if (action == GLFW_RELEASE && key == GLFW_KEY_MINUS)
	{
		loadGame();
		return;
	}

	// save
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

	if (game_screen == GAME_SCREEN_ID::SPLASH || game_screen == GAME_SCREEN_ID::CG)
	{
		// implement
		return;
	}

	// when player is in the level up menu, disable some game inputs
	if (game_is_over)
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

	if (action == GLFW_PRESS && key == GLFW_KEY_E)
	{
		// Get player position for start point
		Entity player = registry.players.entities[0];
		Motion &motion = registry.motions.get(player);
		vec2 start_point = motion.position;

		// Calculate end point in direction of mouse cursor
		vec2 screen_center = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
		vec2 mouse_world_offset = vec2(mouse_pos_x - screen_center.x, mouse_pos_y - screen_center.y);
		vec2 direction = normalize(mouse_world_offset);
		vec2 end_point = start_point + direction * 300.0f; // 300 pixels range

		// Create the electricity effect between these points
		ParticleSystem::createElectricityEffect(start_point, end_point);

		std::cout << "Created electricity effect!" << std::endl;
	}

	// Calculate cell indices
	int cell_x = static_cast<int>(motion.position.x) / GRID_CELL_WIDTH_PX;
	int cell_y = static_cast<int>(motion.position.y) / GRID_CELL_HEIGHT_PX;

	// Kung: Plant seed with the right click button (F button retained for debugging)
	if ((action == GLFW_PRESS && key == GLFW_MOUSE_BUTTON_RIGHT) || (action == GLFW_PRESS && key == GLFW_KEY_F) || (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT))
	{
		plant_seed();
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

	// Add this case in the on_key function where other key inputs are handled
	if (action == GLFW_PRESS && key == GLFW_KEY_SPACE && !player_is_dashing && dash_cooldown_ms <= 0.0f)
	{
		// Get player entity and motion
		Entity player = registry.players.entities[0];
		Motion &motion = registry.motions.get(player);

		// Determine dash direction - use current movement direction or facing direction if not moving
		dash_direction = motion.velocity;
		if (length(dash_direction) < 0.1f) // Not moving, use facing direction
		{
			dash_direction.x = motion.scale.x > 0 ? 1.0f : -1.0f;
			dash_direction.y = 0.0f;
		}
		else
		{
			// Normalize the direction vector
			dash_direction = normalize(dash_direction);
		}

		// Start the dash
		player_is_dashing = true;
		dash_timer_ms = PLAYER_DASH_DURATION_MS;

		// Apply the dash velocity to all moveWithCamera entities
		for (Entity mwc_entity : registry.moveWithCameras.entities)
		{
			if (registry.motions.has(mwc_entity))
			{
				Motion &mwc_motion = registry.motions.get(mwc_entity);

				// Calculate dash velocity based on direction and speed
				vec2 dash_velocity = dash_direction * PLAYER_DASH_SPEED_MULTIPLIER;

				// Apply dash velocity (accounting for normal movement limits)
				if (dash_direction.x > 0)
					mwc_motion.velocity.x = PLAYER_MOVE_RIGHT_SPEED * PLAYER_DASH_SPEED_MULTIPLIER;
				else if (dash_direction.x < 0)
					mwc_motion.velocity.x = PLAYER_MOVE_LEFT_SPEED * PLAYER_DASH_SPEED_MULTIPLIER;

				if (dash_direction.y > 0)
					mwc_motion.velocity.y = PLAYER_MOVE_DOWN_SPEED * PLAYER_DASH_SPEED_MULTIPLIER;
				else if (dash_direction.y < 0)
					mwc_motion.velocity.y = PLAYER_MOVE_UP_SPEED * PLAYER_DASH_SPEED_MULTIPLIER;
			}
		}

		// Play dash sound effect if you have one
		// Mix_PlayChannel(1, dash_sound, 0);
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

	if (action == GLFW_PRESS && key == GLFW_KEY_H)
	{
		// Get player entity
		Entity player = registry.players.entities[0];
		// Set player health to a very high value
		registry.players.get(player).health = 999999999;
		// Also set health bar to 100%
		registry.screenStates.get(registry.screenStates.entities[0]).hp_percentage = 1.0;
		std::cout << "CHEAT ACTIVATED: Player health set to 999999999" << std::endl;
	}

	// key to start challenge
	if (action == GLFW_PRESS && key == GLFW_KEY_B)
	{
		// Debug key to start challenge
		current_day = 5;
	}

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_RIGHT)
			current_seed++;
		else if (key == GLFW_KEY_LEFT)
			current_seed--;
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_9)
	{
		if (registry.screenStates.size() != 0)
		{
			if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage >= 1.0)
			{
				// come back later!
				if (registry.inventorys.components[0].seedCount[current_seed] == 0)
				{
					createSeedInventory(vec2(motion.position.x - TOOLBAR_WIDTH / 2 + TOOLBAR_HEIGHT * (current_seed + 0.5), motion.position.y + CAMERA_VIEW_HEIGHT * 0.45), motion.velocity, current_seed, 0);
				}
				registry.inventorys.components[0].seedCount[current_seed]++; // increment the seed count
				registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
				level++;

				print_level();

				// Get player entity and size
				Entity player = registry.players.entities[0];
				vec2 player_pos = registry.motions.get(player).position;
				vec2 player_size = registry.motions.get(player).scale;
				ParticleSystem::createLevelUpEffect(player_pos, player_size);
				if (level == 2)
				{
					registry.screenStates.components[0].cutscene = 3;
					registry.screenStates.components[0].cg_index = 0;
					return start_cg(renderer);
				}				
			}
			else
			{
				registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += 0.1;
			}
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_1)
	{
		// Debug key to start challenge
		current_seed = 0;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x - 4*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;
	}
		if (action == GLFW_PRESS && key == GLFW_KEY_2)
	{
		// Debug key to start challenge
		current_seed = 1;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x - 3*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_3)
	{
		// Debug key to start challenge
		current_seed = 2;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x - 2*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_4)
	{
		// Debug key to start challenge
		current_seed = 3;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x - 1*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_5)
	{
		// Debug key to start challenge
		current_seed = 4;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x - 0*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_6)
	{
		// Debug key to start challenge
		current_seed = 5;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x + 1*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_7)
	{
		// Debug key to start challenge
		current_seed = 6;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x + 2*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}
		if (action == GLFW_PRESS && key == GLFW_KEY_8)
	{
		// Debug key to start challenge
		current_seed = 7;
		registry.motions.get(registry.toolbars.entities[1]).position.x = registry.motions.get(registry.toolbars.entities[0]).position.x  + 3*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;

	}

}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;

	if (game_screen == GAME_SCREEN_ID::SPLASH || game_screen == GAME_SCREEN_ID::CG)
	{
		return;
	}

	if (game_is_over)
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

void WorldSystem::clearButtons()
{
	for (int i = registry.cgs.entities.size() - 1; i >= 0; i--)
	{
		Entity e = registry.cgs.entities[i];
		registry.remove_all_components_of(e);
	}
}

bool WorldSystem::detectButtons()
{
	for (auto &b : registry.buttons.components)
	{
		if (game_screen == GAME_SCREEN_ID::PLAYING || game_screen == GAME_SCREEN_ID::PAUSE)
		{

			if (mouse_pos_x >= b.position.x - 30 && mouse_pos_x <= b.position.x + 30 &&
				mouse_pos_y >= b.position.y - 30 && mouse_pos_y <= b.position.y + 30)
			{
				if (game_screen == GAME_SCREEN_ID::PLAYING && b.type == BUTTON_ID::PAUSE)
				{
					game_screen = GAME_SCREEN_ID::PAUSE;
					Entity &player = registry.players.entities[0];
					vec2 player_pos = registry.motions.get(player).position;
					createPausePanel(renderer, vec2(player_pos.x, player_pos.y));
					createButton(renderer, BUTTON_ID::LOAD, vec2(player_pos.x, player_pos.y - CAMERA_VIEW_HEIGHT / 4 + 50), vec2(CAMERA_VIEW_WIDTH / 2 + BUTTON_SPLASH_WIDTH, CAMERA_VIEW_HEIGHT / 2 - CAMERA_VIEW_HEIGHT / 4 + 50 + BUTTON_SPLASH_HEIGHT / 2));
					createButton(renderer, BUTTON_ID::SAVE, vec2(player_pos.x, player_pos.y - CAMERA_VIEW_HEIGHT / 4 + 200), vec2(CAMERA_VIEW_WIDTH / 2 + BUTTON_SPLASH_WIDTH, CAMERA_VIEW_HEIGHT / 2 - CAMERA_VIEW_HEIGHT / 4 + 200 + BUTTON_SPLASH_HEIGHT / 2));
					createButton(renderer, BUTTON_ID::QUIT, vec2(player_pos.x, player_pos.y - CAMERA_VIEW_HEIGHT / 4 + 350), vec2(CAMERA_VIEW_WIDTH / 2 + BUTTON_SPLASH_WIDTH, CAMERA_VIEW_HEIGHT / 2 - CAMERA_VIEW_HEIGHT / 4 + 350 + BUTTON_SPLASH_HEIGHT / 2));
					return true;
				}
				else if (game_screen == GAME_SCREEN_ID::PAUSE && b.type == BUTTON_ID::PAUSE)
				{
					game_screen = GAME_SCREEN_ID::PLAYING;
					Entity &player_entity = registry.players.entities[0];
					vec2 player_pos = registry.motions.get(player_entity).position;
					clearButtons();
					createPause(vec2(player_pos.x - CAMERA_VIEW_WIDTH / 2 + 30, player_pos.y - CAMERA_VIEW_HEIGHT / 2 + 30));
					return true;
				}
			}
		}
		// std::cout<<"x "<<b.position.x - BUTTON_SPLASH_WIDTH / 2<<" "<<b.position.x + BUTTON_SPLASH_WIDTH / 2<<std::endl;
		// std::cout<<"y "<<b.position.y - BUTTON_SPLASH_HEIGHT / 2<<" "<<b.position.y + BUTTON_SPLASH_HEIGHT / 2<<std::endl;
		if (mouse_pos_x >= b.position.x - BUTTON_SPLASH_WIDTH / 2 && mouse_pos_x <= b.position.x + BUTTON_SPLASH_WIDTH / 2 &&
			mouse_pos_y >= b.position.y - BUTTON_SPLASH_HEIGHT / 2 && mouse_pos_y <= b.position.y + BUTTON_SPLASH_HEIGHT / 2)
		{
			if (b.type == BUTTON_ID::START)
			{
				registry.screenStates.components[0].cutscene = 1;
				registry.screenStates.components[0].cg_index = 0;
				start_cg(renderer);
			}
			if (b.type == BUTTON_ID::LOAD)
			{
				loadGame();
				game_screen = GAME_SCREEN_ID::PLAYING;
			}
			else if (b.type == BUTTON_ID::TUTORIAL)
			{
				restart_tutorial();
			}
			else if (b.type == BUTTON_ID::QUIT)
			{

				if (game_screen == GAME_SCREEN_ID::SPLASH)
				{
					close_window();
				}
				else
				{
					game_screen = GAME_SCREEN_ID::SPLASH;
					restart_splash_screen();
				}
			}
			else if (b.type == BUTTON_ID::SAVE)
			{
				game_screen == GAME_SCREEN_ID::PLAYING;
				clearButtons();
				saveGame();
				createPause(vec2(30, 30));
			}
			return true;
		}
	}
	return false;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
	if (game_screen == GAME_SCREEN_ID::SPLASH || game_screen == GAME_SCREEN_ID::PAUSE)
	{
		if (action == GLFW_RELEASE && action == GLFW_MOUSE_BUTTON_LEFT)
		{

			std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
			detectButtons();
			return;
		}
	}

	else if (game_screen == GAME_SCREEN_ID::CG)
	{
		if (action == GLFW_RELEASE && action == GLFW_MOUSE_BUTTON_LEFT)
		{
			int cg_index = registry.screenStates.components[0].cg_index++;
			int cutscene = registry.screenStates.components[0].cutscene;
			if (cutscene == 1 && cg_index == 6)
			{
				for (int i = registry.cgs.entities.size() - 1; i >= 0; i--)
					registry.remove_all_components_of(registry.cgs.entities[i]);
				createScreen(renderer, TEXTURE_ASSET_ID::DAY_BG);
				createCharacter(renderer, vec2(WINDOW_WIDTH_PX - 300, WINDOW_HEIGHT_PX - 250), vec2(-500, 500), TEXTURE_ASSET_ID::ORC_WALK2);
				createCharacter(renderer, vec2(200, WINDOW_HEIGHT_PX - 250), vec2(500, 500), TEXTURE_ASSET_ID::PLAYER_IDLE1);
			}
			else if (cutscene == 1 && cg_index == 10)
				restart_game();

			else if (cutscene == 2 && cg_index == 0)
			{
				createCharacter(renderer, vec2(WINDOW_WIDTH_PX - 300, WINDOW_HEIGHT_PX - 250), vec2(200, 200), TEXTURE_ASSET_ID::PLANT_2_IDLE_S);
				createCharacter(renderer, vec2(200, WINDOW_HEIGHT_PX - 250), vec2(500, 500), TEXTURE_ASSET_ID::PLAYER_IDLE1);
			}
			else if (cutscene == 2 && cg_index == 6)
			{
				for (int i = registry.cgs.entities.size() - 1; i >= 0; i--)
					registry.remove_all_components_of(registry.cgs.entities[i]);
				set_game_screen(GAME_SCREEN_ID::PLAYING);
			}
			else if (cutscene == 3 && cg_index == 0)
			{
				createCharacter(renderer, vec2(WINDOW_WIDTH_PX - 300, WINDOW_HEIGHT_PX - 250), vec2(200, 200), TEXTURE_ASSET_ID::CHICKEN_CG);
				createCharacter(renderer, vec2(200, WINDOW_HEIGHT_PX - 250), vec2(500, 500), TEXTURE_ASSET_ID::PLAYER_IDLE1);
			}
			else if (cutscene == 3 && cg_index == 5)
			{
				for (int i = registry.cgs.entities.size() - 1; i >= 0; i--)
					registry.remove_all_components_of(registry.cgs.entities[i]);
				set_game_screen(GAME_SCREEN_ID::PLAYING);
			}
		}

		return;
	}

	else if (!WorldSystem::game_is_over)
	{
		// on button press
		if (action == GLFW_PRESS)
		{

			int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
			int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

			std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
		}

		if (action == GLFW_RELEASE)
		{
			if (!detectButtons()) {
				if (button == GLFW_MOUSE_BUTTON_LEFT)
					player_attack();
				else if (button == GLFW_MOUSE_BUTTON_RIGHT)
					plant_seed();
			}
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
	current_day++; spawn_manager.set_day(current_day);
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
	if (DAY_MAP.find(day) != DAY_MAP.end()) {
		int total = 0;
		for (const auto& [enemy, count] : DAY_MAP.at(day))
			total += count;
		return total;
	}

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
		if (rest_timer_ms < DAY_DELAY_MS)
		{ // 10 second rest
			// Optional: Display countdown text
			float remaining = (DAY_DELAY_MS - rest_timer_ms) / 1000.f;
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

void WorldSystem::loadGame()
{
	registry.clear_all_components();
	game_screen = GAME_SCREEN_ID::PLAYING;
	json jsonFile;
	std::ifstream file(PROJECT_SOURCE_DIR + std::string("data/reload/game_0.json"));
	file >> jsonFile;
	game_is_over = jsonFile["game_is_over"];
	game_screen = jsonFile["game_screen"];
	current_day = jsonFile["current_day"];
	current_seed = jsonFile["current_seed"];
	level = jsonFile["level"];
	Entity::overrideIDCount((int)jsonFile["id_count"]);

	json ss_json = jsonFile["0"][0];
	ScreenState &ss = registry.screenStates.components[0];
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
	ss.cg_index = ss_json["cg_index"];
	ss.cutscene = ss_json["cutscene"];
	ss.seed_cg = ss_json["seed_cg"];

	json attack_arr = jsonFile["1"];
	for (long unsigned int i = 0; i < attack_arr.size(); i++)
	{
		json attack_json = attack_arr[i];
		Entity e = Entity(attack_json["entity"]);
		Attack &attack = registry.attacks.emplace(e);
		attack.range = attack_json["range"];
		attack.damage = attack_json["damage"];
	}

	json motion_arr = jsonFile["2"];
	for (long unsigned int i = 0; i < motion_arr.size(); i++)
	{
		json motion = motion_arr[i];
		Entity e = Entity(motion["entity"]);
		Motion &m = registry.motions.emplace(e);
		m.position = vec2(motion["position"][0], motion["position"][1]);
		m.angle = motion["angle"];
		m.velocity = vec2(0.0, 0.0);
		m.scale = vec2(motion["scale"][0], motion["scale"][1]);
	}

	json collisions_arr = jsonFile["3"];
	for (long unsigned int i = 0; i < collisions_arr.size(); i++)
	{
		json collision = collisions_arr[i];
		Entity e = Entity(collision["entity"].get<int>());
		Entity other = Entity(collision["other"].get<int>());
		registry.collisions.emplace(e, other);
	}

	// didnt add meshPtrs, maybe add constraints when chicken summoned cannot save lol

	json dimension_arr = jsonFile["5"];
	for (long unsigned int i = 0; i < dimension_arr.size(); i++)
	{
		json dimension_json = dimension_arr[i];
		Entity e = Entity(dimension_json["entity"]);
		Dimension &dimension = registry.dimensions.emplace(e);
		dimension.height = dimension_json["height"];
		dimension.width = dimension_json["width"];
	}

	json renderRequests_arr = jsonFile["6"];
	for (long unsigned int i = 0; i < renderRequests_arr.size(); i++)
	{
		json rr_json = renderRequests_arr[i];
		Entity e = Entity(rr_json["entity"]);
		RenderRequest &rr = registry.renderRequests.emplace(e);
		rr.used_texture = (TEXTURE_ASSET_ID)rr_json["used_texture"];
		rr.used_effect = (EFFECT_ASSET_ID)rr_json["used_effect"];
		rr.used_geometry = (GEOMETRY_BUFFER_ID)rr_json["used_geometry"];
	}

	json tower_arr = jsonFile["8"];
	for (long unsigned int i = 0; i < tower_arr.size(); i++)
	{
		json tower_json = tower_arr[i];
		Entity e = Entity(tower_json["entity"]);
		Tower &tower = registry.towers.emplace(e);
		tower.health = tower_json["health"];
		tower.damage = tower_json["damage"];
		tower.range = tower_json["range"];
		tower.timer_ms = tower_json["timer_ms"];
		tower.state = tower_json["state"];
	}

	json zombie_arr = jsonFile["10"];
	for (long unsigned int i = 0; i < zombie_arr.size(); i++)
	{
		json zombie_json = zombie_arr[i];
		Entity e = Entity(zombie_json["entity"]);
		Zombie &zombie = registry.zombies.emplace(e);
		zombie.health = zombie_json["health"];
	}

	json zombieSpawn_arr = jsonFile["11"];
	for (long unsigned int i = 0; i < zombieSpawn_arr.size(); i++)
	{
		json zombieSpawn_json = zombieSpawn_arr[i];
		Entity e = Entity(zombieSpawn_json["entity"]);
		ZombieSpawn &zombieSpawn = registry.zombieSpawns.emplace(e);
	}

	json player_arr = jsonFile["12"];
	for (long unsigned int i = 0; i < player_arr.size(); i++)
	{
		json player_json = player_arr[i];
		Entity e = Entity(player_json["entity"]);
		Player &player = registry.players.emplace(e);
		player.health = player_json["health"];
	}

	json sc_arr = jsonFile["13"];
	for (long unsigned int i = 0; i < sc_arr.size(); i++)
	{
		json sc_json = sc_arr[i];
		Entity e = Entity(sc_json["entity"]);
		StatusComponent &sc = registry.statuses.emplace(e);
		for (const auto &s : sc_json["active_statuses"])
		{
			Status status;
			status.type = s["type"];
			status.duration_ms = s["duration_ms"];
			status.value = s["value"];
			sc.active_statuses.push_back(status);
		}
	}

	json states_arr = jsonFile["14"];
	for (long unsigned int i = 0; i < states_arr.size(); i++)
	{
		json state_json = states_arr[i];
		Entity e = Entity(state_json["entity"]);
		State &state = registry.states.emplace(e);
		state.state = (STATE)state_json["state"];
	}

	json animation_arr = jsonFile["15"];
	for (long unsigned int i = 0; i < animation_arr.size(); i++)
	{
		json animation_json = animation_arr[i];
		Entity e = Entity(animation_json["entity"]);
		Animation &animation = registry.animations.emplace(e);
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
	for (long unsigned int i = 0; i < death_arr.size(); i++)
	{
		json death_json = death_arr[i];
		Entity e = Entity(death_json["entity"]);
		Death &death = registry.deaths.emplace(e);
	}

	json cooldown_arr = jsonFile["17"];
	for (long unsigned int i = 0; i < cooldown_arr.size(); i++)
	{
		json cooldown_json = cooldown_arr[i];
		Entity e = Entity(cooldown_json["entity"]);
		Cooldown &cooldown = registry.cooldowns.emplace(e);
		cooldown.timer_ms = cooldown_json["timer_ms"];
	}

	json da_arr = jsonFile["18"];
	for (long unsigned int i = 0; i < da_arr.size(); i++)
	{
		json da_json = da_arr[i];
		Entity e = Entity(da_json["entity"]);
		DeathAnimation &da = registry.deathAnimations.emplace(e);
		da.slide_direction = vec2(da_json["slide_direction"][0], da_json["slide_direction"][1]);
		da.alpha = da_json["alpha"];
		da.duration_ms = da_json["duration_ms"];
	}

	json he_arr = jsonFile["19"];
	for (long unsigned int i = 0; i < he_arr.size(); i++)
	{
		json he_json = he_arr[i];
		Entity e = Entity(he_json["entity"]);
		HitEffect &he = registry.hitEffects.emplace_with_duplicates(e);
		he.duration_ms = he_json["duration_ms"];
		he.is_white = he_json["is_white"];
	}

	json projectile_arr = jsonFile["20"];
	for (long unsigned int i = 0; i < projectile_arr.size(); i++)
	{
		json projectile_json = projectile_arr[i];
		Entity e = Entity(projectile_json["entity"]);
		Entity source = Entity(projectile_json["source"]);
		Projectile &p = registry.projectiles.emplace(e);
		p.source = source;
		p.damage = projectile_json["damage"];
		p.speed = projectile_json["speed"];
		p.lifetime_ms = projectile_json["lifetime_ms"];
		p.direction = vec2(projectile_json["direction"][0], projectile_json["direction"][1]);
		p.invincible = projectile_json["invincible"];
	}

	json camera_arr = jsonFile["21"];
	for (long unsigned int i = 0; i < camera_arr.size(); i++)
	{
		json camera_json = camera_arr[i];
		Entity e = Entity(camera_json["entity"]);
		Camera &camera = registry.cameras.emplace(e);
		camera.position = vec2(camera_json["position"][0], camera_json["position"][1]);
		camera.camera_width = camera_json["camera_width"];
		camera.camera_height = camera_json["camera_height"];
		camera.lerp_factor = camera_json["lerp_factor"];
	}

	json skeleton_arr = jsonFile["22"];
	for (long unsigned int i = 0; i < skeleton_arr.size(); i++)
	{
		json skeleton_json = skeleton_arr[i];
		Entity e = Entity(skeleton_json["entity"]);
		Entity target = Entity(skeleton_json["target"]);
		Skeleton &skeleton = registry.skeletons.emplace(e);
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
	for (long unsigned int i = 0; i < arrow_arr.size(); i++)
	{
		json arrow_json = arrow_arr[i];
		Entity e = Entity(arrow_json["entity"]);
		Entity source = Entity(arrow_json["source"]);
		Arrow &a = registry.arrows.emplace(e);
		a.source = source;
		a.damage = arrow_json["damage"];
		a.speed = arrow_json["speed"];
		a.lifetime_ms = arrow_json["lifetime_ms"];
		a.direction = vec2(arrow_json["direction"][0], arrow_json["direction"][1]);
	}

	json visualScale_arr = jsonFile["24"];
	for (long unsigned int i = 0; i < visualScale_arr.size(); i++)
	{
		json visualScale_json = visualScale_arr[i];
		Entity e = Entity(visualScale_json["entity"]);
		VisualScale &vs = registry.visualScales.emplace(e);
		vs.scale = vec2(visualScale_json["scale"][0], visualScale_json["scale"][1]);
	}

	json enemies_arr = jsonFile["25"];
	for (long unsigned int i = 0; i < enemies_arr.size(); i++)
	{
		json enemies_json = enemies_arr[i];
		Entity e = Entity(enemies_json["entity"]);
		Enemy &enemy = registry.enemies.emplace(e);
		enemy.health = enemies_json["health"];
		enemy.speed = enemies_json["speed"];
	}

	json inventory_arr = jsonFile["26"];
	for (long unsigned int i = 0; i < inventory_arr.size(); i++)
	{
		json inventory_json = inventory_arr[i];
		Entity e = Entity(inventory_json["entity"]);
		Inventory &in = registry.inventorys.emplace(e);
		json seed_arr = inventory_json["seedCount"];
		for (long unsigned int i = 0; i < seed_arr.size(); i++)
		{
			in.seedCount[i] = seed_arr[std::to_string(i)];
		}
	}

	json seed_arr = jsonFile["27"];
	for (long unsigned int i = 0; i < seed_arr.size(); i++)
	{
		json seed_json = seed_arr[i];
		Entity e = Entity(seed_json["entity"]);
		Seed &seed = registry.seeds.emplace(e);
		seed.type = seed_json["type"];
		seed.timer = seed_json["timer"];
	}

	json mvc_arr = jsonFile["28"];
	for (long unsigned int i = 0; i < mvc_arr.size(); i++)
	{
		json mvc_json = mvc_arr[i];
		Entity e = Entity(mvc_json["entity"]);
		registry.moveWithCameras.emplace(e);
	}

	json mt_arr = jsonFile["29"];
	for (long unsigned int i = 0; i < mt_arr.size(); i++)
	{
		json mt_json = mt_arr[i];
		Entity e = Entity(mt_json["entity"]);
		registry.mapTiles.emplace(e);
	}

	json plant_animation_arr = jsonFile["35"];
	for (long unsigned int i = 0; i < plant_animation_arr.size(); i++)
	{
		json plant_animation_json = plant_animation_arr[i];
		Entity e = Entity(plant_animation_json["entity"]);
		PlantAnimation& plant_animation = registry.plantAnimations.emplace(e);
		plant_animation.id = plant_animation_json["id"];
	}

	Entity& player_entity = registry.players.entities[0];
	vec2 player_pos = registry.motions.get(player_entity).position;
	clearButtons();
	createPause(vec2(player_pos.x - CAMERA_VIEW_WIDTH/2+30, player_pos.y - CAMERA_VIEW_HEIGHT/2+30));

	std::cout << "Game loaded successfully." << std::endl;
}

void WorldSystem::saveGame()
{
	if (chicken_summoned)
	{
		std::cout << "Chicken summoned, cannot save, please give it some time to fly." << std::endl;
		return;
	}
	if (game_screen == GAME_SCREEN_ID::CG)
	{
		std::cout << "Finish the cutscene before trying to save." << std::endl;
		return;
	}
	json jsonFile;
	jsonFile["game_is_over"] = game_is_over;
	jsonFile["game_screen"] = game_screen;
	jsonFile["current_day"] = current_day;
	jsonFile["current_seed"] = current_seed;
	jsonFile["level"] = level;
	jsonFile["id_count"] = Entity::get_id_count();

	clearButtons();
	game_screen = GAME_SCREEN_ID::PLAYING;

	for (int i = 0; i < registry.registry_list.size(); i++)
	{
		jsonFile[std::to_string(i)] = registry.registry_list[i]->toJSON();
	}

	std::ofstream outFile(PROJECT_SOURCE_DIR + std::string("data/reload/game_0.json"));
	if (outFile.is_open())
	{
		outFile << jsonFile.dump(4);
		outFile.close();
		std::cout << "JSON written to file successfully.\n";
	}
	else
	{
		std::cerr << "Error opening file for writing.\n";
	}
}
 

void WorldSystem::plant_seed()
{
	// Get player's motion component
	Entity player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);

	// Calculate player's current cell for proximity checking
	int cell_x = static_cast<int>((motion.position.x + GRID_CELL_WIDTH_PX / 2) / GRID_CELL_WIDTH_PX);
	int cell_y = static_cast<int>((motion.position.y + GRID_CELL_HEIGHT_PX / 2) / GRID_CELL_HEIGHT_PX);
	vec2 grid_center = vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX);

	// Find valid farmland closest to the player
	Entity closest_farmland = NULL;
	float closest_distance = GRID_CELL_WIDTH_PX; // Max distance to consider

	for (Entity tile : registry.mapTiles.entities)
	{
		// Check if tile is farmland
		if (registry.motions.has(tile) &&
			registry.renderRequests.has(tile) &&
			registry.renderRequests.get(tile).used_texture == DECORATION_LIST[6])
		{
			vec2 farmland_pos = registry.motions.get(tile).position;
			float distance = length(farmland_pos - grid_center);

			// Check if this is closer than previous matches
			if (distance < closest_distance)
			{
				// Check if tile is already occupied by a seed or tower
				bool is_occupied = false;
				for (Entity entity : registry.motions.entities)
				{
					if ((registry.seeds.has(entity) || registry.towers.has(entity)) &&
						length(registry.motions.get(entity).position - farmland_pos) < 10.0f)
					{
						is_occupied = true;
						break;
					}
				}

				if (!is_occupied)
				{
					closest_farmland = tile;
					closest_distance = distance;
				}
			}
		}
	}

	// If we found valid farmland, plant a seed
	if (closest_farmland != NULL)
	{
		vec2 farmland_pos = registry.motions.get(closest_farmland).position;

		// Check if player has seeds available
		if (registry.inventorys.components[0].seedCount[current_seed] > 0)
		{
			// Plant the seed at the exact farmland position
			Entity seed = createSeed(farmland_pos, current_seed);
			registry.inventorys.components[0].seedCount[current_seed]--;

			// If that was the last seed of this type, remove it from toolbar
			if (registry.inventorys.components[0].seedCount[current_seed] == 0)
			{
				for (Entity seed_entity : registry.seeds.entities)
				{
					if (registry.moveWithCameras.has(seed_entity) &&
						registry.seeds.get(seed_entity).type == current_seed)
					{
						registry.remove_all_components_of(seed_entity);
						break;
					}
				}
			}

			std::cout << "Planted seed. Remaining: " << registry.inventorys.components[0].seedCount[current_seed] << std::endl;
		}
		else
		{
			std::cout << "No seeds of this type available!" << std::endl;
		}
	}
	else
	{
		// No valid farmland found
		std::cout << "No available farmland nearby. Move closer to farmland." << std::endl;
	}
}

void WorldSystem::update_dash(float elapsed_ms_since_last_update)
{
	if (player_is_dashing)
	{
		dash_timer_ms -= elapsed_ms_since_last_update;

		if (dash_timer_ms <= 0)
		{
			// End the dash
			player_is_dashing = false;
			dash_timer_ms = 0.0f;
			dash_cooldown_ms = PLAYER_DASH_COOLDOWN_MS;

			// Reset velocities to zero
			for (Entity mwc_entity : registry.moveWithCameras.entities)
			{
				if (registry.motions.has(mwc_entity))
				{
					Motion &mwc_motion = registry.motions.get(mwc_entity);
					mwc_motion.velocity = vec2(0.0f, 0.0f);

					// Re-apply velocity for any keys that are still being pressed
					if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
						mwc_motion.velocity.y += PLAYER_MOVE_UP_SPEED;
					if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
						mwc_motion.velocity.y += PLAYER_MOVE_DOWN_SPEED;
					if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
						mwc_motion.velocity.x += PLAYER_MOVE_LEFT_SPEED;
					if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
						mwc_motion.velocity.x += PLAYER_MOVE_RIGHT_SPEED;
				}
			}
		}
	}

	// Handle dash cooldown
	if (dash_cooldown_ms > 0)
	{
		dash_cooldown_ms -= elapsed_ms_since_last_update;
		if (dash_cooldown_ms < 0)
			dash_cooldown_ms = 0;
	}
}
