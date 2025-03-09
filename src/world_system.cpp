// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <thread>

#include "physics_system.hpp"
#include "spawn_manager.hpp"
#include "state_system.hpp"

// FreeType
// #include <ft2build.h>
// #include FT_FREETYPE_H

// FT_Library library;

bool WorldSystem::game_is_over = false;
Mix_Chunk *WorldSystem::game_over_sound = nullptr;

// create the world
WorldSystem::WorldSystem() : points(0), level(1), game_screen(GAME_SCREEN_ID::PLAYING)
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
	std::cout << "Starting music..." << std::endl;

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
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

	if (registry.zombies.size() == 0 && current_bgm != night_bgm)
	{
		current_bgm = night_bgm;
		std::thread music_thread([this]()
								 {
			// Mix_FadeOutMusic(1000);
			Mix_HaltMusic();
			Mix_FadeInMusic(night_bgm, -1, 1000); });
		music_thread.detach();
	}
	else if (registry.zombies.size() > 0 && current_bgm != combat_bgm)
	{
		current_bgm = combat_bgm;
		std::thread music_thread([this]()
								 {
			// Mix_FadeOutMusic(1000);
			Mix_HaltMusic();
			Mix_FadeInMusic(combat_bgm, -1, 1000); });
		music_thread.detach();
	}

	if (!WorldSystem::game_is_over)
	{
		update_camera();
		spawn_manager.step(elapsed_ms_since_last_update, renderer);

		update_enemy_death_animations(elapsed_ms_since_last_update);
		update_movement_sound(elapsed_ms_since_last_update);
		update_screen_shake(elapsed_ms_since_last_update);
		return true;
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	registry.clear_all_components();
	std::cout << "Restarting..." << std::endl;
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
	level = 1;
	game_screen = GAME_SCREEN_ID::PLAYING;
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
	//commented out Kung's code
	for (int x = -SCORCHED_EARTH_DIMENSION_PX * 4; x < MAP_WIDTH_PX + SCORCHED_EARTH_DIMENSION_PX * 4; x += SCORCHED_EARTH_DIMENSION_PX)
	{
		for (int y = -SCORCHED_EARTH_DIMENSION_PX * 2; y < MAP_HEIGHT_PX + SCORCHED_EARTH_DIMENSION_PX * 2; y += SCORCHED_EARTH_DIMENSION_PX)
		{
			createScorchedEarth(vec2(x, y));
		}
	}
	// Kung: This is for Milestone #2. This creates the farmland.
	parseMap(true);
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

	// if the screenState exists, reset the health bar percentages
	if (registry.screenStates.size() != 0)
	{
		registry.screenStates.get(registry.screenStates.entities[0]).hp_percentage = 1.0;
		registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
	}

	// create the tutorial assets
	createTutorialMove(vec2(TUTORIAL_WIDTH_PX * 0.2, WINDOW_HEIGHT_PX * -0.25));
	createTutorialAttack(vec2(TUTORIAL_WIDTH_PX * 0.4, WINDOW_HEIGHT_PX * -0.25));
	createTutorialPlant(vec2(TUTORIAL_WIDTH_PX * 0.6, WINDOW_HEIGHT_PX * -0.25));
	createTutorialRestart(vec2(TUTORIAL_WIDTH_PX * 0.8, WINDOW_HEIGHT_PX * -0.25));

	// create the arrows for the tutorial
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 4, TUTORIAL_HEIGHT_PX / 2));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX / 2, TUTORIAL_HEIGHT_PX / 2));
	createTutorialArrow(vec2(TUTORIAL_WIDTH_PX * 0.75, TUTORIAL_HEIGHT_PX / 2));

	// reset player and spawn player in the middle of the screen
	createPlayer(renderer, vec2{WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});
	
	// reset camera position
	createCamera(renderer, vec2{WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});

	// Kung: Create the pause button and toolbar, and have them overlay the player
	// registry.pauses.clear();
	registry.toolbars.clear();
	// createPause();
	createToolbar();

	// Kung: Reset player movement so that the player remains still when no keys are pressed

	// Move left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
        }
	}

	// Move right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
        }
	}

	// Move down
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
        }
	}

	// Move up
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
        }
	}

	// start the spawn manager
	spawn_manager.start_game();

	// Print the starting level (Level 1)
	std::cout << "==== LEVEL " << level << " ====" << std::endl;
}

// Compute collisions between entities
void WorldSystem::handle_collisions()
{
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
	if (!registry.cooldowns.has(player))
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

		// Slash Animation
		createEffect(renderer, weapon_motion.position, weapon_motion.scale);

		for (int i = 0; i < registry.zombies.size(); i++)
		{
			if (PhysicsSystem::collides(weapon_motion, registry.motions.get(registry.zombies.entities[i])) // if zombie and weapon collide, decrease zombie health
				|| PhysicsSystem::collides(registry.motions.get(registry.players.entities[0]), registry.motions.get(registry.zombies.entities[i])))
			{
				Entity zombie = registry.zombies.entities[i];
				if (registry.zombies.has(zombie))
				{
					auto &zombie_comp = registry.zombies.get(zombie);
					zombie_comp.health -= registry.attacks.get(registry.players.entities[0]).damage;
					std::cout << "wow u r attacking so nice cool cool" << std::endl;

					// Calculate knockback direction (from player to zombie)
					Motion &zombie_motion = registry.motions.get(zombie);
					Motion &player_motion = registry.motions.get(player);
					vec2 direction = zombie_motion.position - player_motion.position;
					float length = sqrt(dot(direction, direction));
					if (length > 0)
					{
						direction = direction / length; // Normalize
					}

					// Apply knockback velocity immediately
					float knockback_force = 1000.0f;
					zombie_motion.velocity += direction * knockback_force;

					// Add hit effect
					HitEffect &hit = registry.hitEffects.emplace_with_duplicates(zombie);

					// This is what you do when you kill a zombie.
					if (zombie_comp.health <= 0)
					{
						// Add death animation before removing
						Entity player = registry.players.entities[0];
						Motion &player_motion = registry.motions.get(player);
						vec2 slide_direction = {player_motion.scale.x > 0 ? 1.0f : -1.0f, 0.0f};

						// Add death animation component
						DeathAnimation &death_anim = registry.deathAnimations.emplace(zombie);
						death_anim.slide_direction = slide_direction;
						death_anim.alpha = 1.0f;
						death_anim.duration_ms = 500.0f; // Animation lasts 0.5 seconds

            			// Increase the counter that represents the number of zombies killed.
						points++;
						std::cout<<"Zombies killed: "<<points<<std::endl;
            
						// Kung: Upon killing a zombie, increase the experience of the player or reset the experience bar when it becomes full.
						if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage < 1.0)
						{
							registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += registry.attacks.get(registry.players.entities[0]).damage / PLAYER_HEALTH;
						} // Kung: If the bar is full, reset the player experience bar and upgrade the user level.
						else if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage >= 1.0)
						{
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
		if (registry.cooldowns.has(player)) {
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

			// Kung: Upon killing a zombie, update the enemy count and print it to the console.
			std::cout << "Enemy count: " << registry.zombies.size() << " zombies" << std::endl;
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
void WorldSystem::player_movement(int key, int action, Motion& player_motion) {
	// Move left
	if (player_motion.position.x >= PLAYER_LEFT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_LEFT_SPEED;
			}
		}
	}

	// Move right
	if (player_motion.position.x <= PLAYER_RIGHT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_RIGHT_SPEED;
			}
		}
	}

	// Move down
	if (player_motion.position.y <= PLAYER_DOWN_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_DOWN_SPEED;
			}
		}
	}

	// Move up
	if (player_motion.position.y >= PLAYER_UP_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_UP_SPEED;
			}
		}
	}
}

// Version of player_movement that works for the tutorial mode.
void WorldSystem::player_movement_tutorial(int key, int action, Motion& player_motion) {
	// Move left
	if (player_motion.position.x >= PLAYER_LEFT_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_LEFT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_A)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_LEFT_SPEED;
			}
		}
	}

	// Move right
	if (player_motion.position.x <= PLAYER_RIGHT_BOUNDARY_TUTORIAL)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x += PLAYER_MOVE_RIGHT_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_D)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x -= PLAYER_MOVE_RIGHT_SPEED;
			}
		}
	}

	// Move down
	if (player_motion.position.y <= PLAYER_DOWN_BOUNDARY_TUTORIAL)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_DOWN_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_S)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_DOWN_SPEED;
			}
		}
	}

	// Move up
	if (player_motion.position.y >= PLAYER_UP_BOUNDARY)
	{
		if (action == GLFW_PRESS && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y += PLAYER_MOVE_UP_SPEED;
			}
		}
		else if (action == GLFW_RELEASE && key == GLFW_KEY_W)
		{
			for (Entity mwc_entity : registry.moveWithCameras.entities) {
				if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y -= PLAYER_MOVE_UP_SPEED;
			}
		}
	}
}

void WorldSystem::on_key(int key, int, int action, int mod)
{
	// Player movement
	Entity player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);

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

		// if (game_screen == GAME_SCREEN_ID::TUTORIAL) {
		// } else {
		// }

		restart_game();
		return;
	}

	// Manual wave generation with 'g'
	if (action == GLFW_PRESS && key == GLFW_KEY_G)
	{
		spawn_manager.generate_wave(renderer);
		return;
	}

	// test mode with 't'
	if (action == GLFW_PRESS && key == GLFW_KEY_T)
	{
		test_mode = !test_mode;
		spawn_manager.set_test_mode(test_mode);
		if (game_screen != GAME_SCREEN_ID::TEST) {
			game_screen = GAME_SCREEN_ID::TEST;
		} else game_screen = GAME_SCREEN_ID::PLAYING;
		std::cout << "Game " << (test_mode ? "entered" : "exited") << " test mode" << std::endl;
		return;
	}

	// tutorial mode with '/'
	if (action == GLFW_PRESS && key == GLFW_KEY_SLASH)
	{
		tutorial_mode = !tutorial_mode;
		spawn_manager.set_test_mode(tutorial_mode);

		// Remove seeds
		for (Entity seed_entity : registry.seeds.entities) {
			registry.remove_all_components_of(seed_entity);
		}

		if (game_screen != GAME_SCREEN_ID::TUTORIAL) {
			game_screen = GAME_SCREEN_ID::TUTORIAL;
		} else {
			game_screen = GAME_SCREEN_ID::PLAYING;
		}
		std::cout << "Game " << (tutorial_mode ? "entered" : "exited") << " test mode" << std::endl;
		return;
	}

	// Calculate cell indices
	int cell_x = static_cast<int>(motion.position.x) / GRID_CELL_WIDTH_PX;
	int cell_y = static_cast<int>(motion.position.y) / GRID_CELL_HEIGHT_PX;

	// Kung: Plant seed with the 'F' button
	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		// You can only plant where there is farmland.
		for (Entity maptile_entity : registry.mapTiles.entities) {
			if (registry.motions.has(maptile_entity) && registry.renderRequests.has(maptile_entity)) {
				if (registry.renderRequests.get(maptile_entity).used_texture == DECORATION_LIST[6]) {
					if (registry.motions.get(maptile_entity).position == vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX)) {
						// Remove any seeds that have already been planted to begin with.
						for (Entity entity : registry.seeds.entities) {
							if (registry.motions.has(entity)) {
								if (registry.motions.get(entity).position == vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX)) {
									registry.motions.remove(entity);
									registry.seeds.remove(entity);
								}
							}
						}
						createSeed(vec2(cell_x * GRID_CELL_WIDTH_PX, cell_y * GRID_CELL_HEIGHT_PX));
					}
				}
			}
		}
	}

	// Haonan: Shoot towers with the 'H' button
	if (action == GLFW_PRESS && key == GLFW_KEY_H)
	{
		// Calculate center position of the cell
		vec2 cell_center = {
			(cell_x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f),
			(cell_y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f)};

		// Create tower at cell center
		// Check if cell is already occupied by a tower
		bool cell_occupied = false;
		for (Entity tower : registry.towers.entities)
		{
			if (!registry.motions.has(tower))
			{
				continue;
			}

			Motion &tower_motion = registry.motions.get(tower);
			int tower_cell_x = static_cast<int>(tower_motion.position.x) / GRID_CELL_WIDTH_PX;
			int tower_cell_y = static_cast<int>(tower_motion.position.y) / GRID_CELL_HEIGHT_PX;

			if (tower_cell_x == cell_x && tower_cell_y == cell_y)
			{
				cell_occupied = true;
				std::cout << "Cell already occupied by a tower!" << std::endl;
				break;
			}
		}


		// Only create tower if cell is empty
		if (!cell_occupied)
		{
			createTower(renderer, cell_center);
		}
	}

	// Kung: Helper function for player movement (see above for description)
	if(game_screen == GAME_SCREEN_ID::TUTORIAL) {
		player_movement_tutorial(key, action, motion);
	} else {
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

	// Debug
	if (action == GLFW_PRESS && key == GLFW_KEY_L)
	{
		registry.list_all_components();
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;

	// change player facing direction
	Entity player = registry.players.entities[0];
	Motion &motion = registry.motions.get(player);

	// face left
	if (mouse_pos_x < WINDOW_WIDTH_PX / 2 && motion.scale.x > 0)
	{
		motion.scale.x = -motion.scale.x;
		// change the positions of detection lines
		//  int id = registry.gridLines.getEntityId(player);
		//  for (int i=0; i<3; i++) {
		//  	GridLine& line = registry.gridLines.getByIndex(id-i);
		//  	line.start_pos.x -= GRID_CELL_WIDTH_PX;
		//  }
	}

	// face right
	if (mouse_pos_x > WINDOW_WIDTH_PX / 2 && motion.scale.x < 0)
	{
		motion.scale.x = -motion.scale.x;
		// change the positions of detection lines
		//  int id = registry.gridLines.getEntityId(player);
		//  for (int i=0; i<3; i++) {
		//  	GridLine& line = registry.gridLines.getByIndex(id-i);
		//  	line.start_pos.x += GRID_CELL_WIDTH_PX;
		//  }
	}
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
	if (!WorldSystem::game_is_over)
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