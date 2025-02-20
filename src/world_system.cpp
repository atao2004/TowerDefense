// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"
#include "spawn_manager.hpp"
#include "state_system.hpp"

// Game constants
bool WorldSystem::game_is_over = false;

// create the world
WorldSystem::WorldSystem() : points(0),
							 max_zombies(MAX_ZOMBIES),
							 next_zombie_spawn(0),
							 zombie_spawn_rate_ms(ZOMBIE_SPAWN_RATE_MS)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());

	if (background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str());
		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{

	// // spawn new zombies
	// next_zombie_spawn -= elapsed_ms_since_last_update * current_speed;
	// if (next_zombie_spawn < 0.f && registry.zombies.size() < max_zombies)
	// {

	// 	// reset timer
	// 	next_zombie_spawn = (ZOMBIE_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ZOMBIE_SPAWN_RATE_MS / 2);

	// 	// create zombie with random initial position
	// 	createZombie(renderer, vec2(50.f + uniform_dist(rng) * (WINDOW_WIDTH_PX - 100.f), 100.f));
	// }

	// Using the spawn manager to generate zombies
	if(WorldSystem::game_is_over) {
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];
		// if (screen.game_over)
		// {
		// 	screen.lerp_timer += elapsed_ms_since_last_update;
		// }
		// if(screen.lerp_timer == 1) {
		// 	screen.lerp_timer = 1;
		// }
	}
	spawn_manager.step(elapsed_ms_since_last_update, renderer);

	update_enemy_death_animations(elapsed_ms_since_last_update);
	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	game_is_over = false;

	// Reset the spawn manager
	spawn_manager.reset();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	max_zombies = MAX_ZOMBIES;
	next_zombie_spawn = 0;
	zombie_spawn_rate_ms = ZOMBIE_SPAWN_RATE_MS;
	registry.screenStates.get(registry.screenStates.entities[0]).game_over = false;
	registry.screenStates.get(registry.screenStates.entities[0]).lerp_timer = 0.0;

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	int grid_line_width = GRID_LINE_WIDTH_PX;

	// create the grass texture for the background and reset the pre-existing grasses
	removeGrasses();
	for (int x = (GRASS_DIMENSION_PX / 2); x < WINDOW_WIDTH_PX + (GRASS_DIMENSION_PX / 2); x += GRASS_DIMENSION_PX) {
		for (int y = (GRASS_DIMENSION_PX / 2); y < WINDOW_HEIGHT_PX + (GRASS_DIMENSION_PX / 2); y += GRASS_DIMENSION_PX) {
			createGrass(vec2(x, y));
		}
	}
	
	// create grid lines and clear any pre-existing grid lines
	grid_lines.clear();
	// vertical lines
	for (int col = 0; col <= WINDOW_WIDTH_PX / GRID_CELL_WIDTH_PX; col++)
	{
		// width of 2 to make the grid easier to see
		grid_lines.push_back(createGridLine(vec2(col * GRID_CELL_WIDTH_PX, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
	}

	// horizontal lines
	for (int row = 0; row <= WINDOW_HEIGHT_PX / GRID_CELL_HEIGHT_PX; row++)
	{
		// width of 2 to make the grid easier to see
		grid_lines.push_back(createGridLine(vec2(0, row * GRID_CELL_HEIGHT_PX), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
	}

	// if the screenState exists, reset the health bar percentages
	if (registry.screenStates.size() != 0) {
		registry.screenStates.get(registry.screenStates.entities[0]).hp_percentage = 1.0;
		registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage = 0.0;
	}

	// Create the pause button and toolbar
	createPause();
	createToolbar();

	// spawn player in the middle of the screen
	createPlayer(renderer, vec2{WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2});

	// start the spawn manager
	spawn_manager.start_game();
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

void WorldSystem::player_attack()
{
	Entity player = registry.players.entities[0];
	if (!registry.cooldowns.has(player))
	{
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
		for (int i = 0; i < registry.zombies.size(); i++)
		{
			// if (PhysicsSystem::collides(weapon_motion, registry.motions.get(registry.zombies.entities[i])))
			// { // if zombie and player weapon collide, decrease zombie health
			// 	Zombie currZombie = registry.zombies.get(registry.zombies.entities[i]);
			// 	std::cout << "wow u r attacking so nice cool cool" << std::endl;
			// 	registry.zombies.get(registry.zombies.entities[i]).health -= registry.attacks.get(registry.players.entities[0]).damage;
			// 	if (registry.zombies.get(registry.zombies.entities[i]).health <= 0)
			// 	{ // if zombie health is below 0, remove him
			// 		registry.remove_all_components_of(registry.zombies.entities[i]);
			// 	}
			// }

			if (PhysicsSystem::collides(weapon_motion, registry.motions.get(registry.zombies.entities[i])) // if zombie and weapon collide, decrease zombie health
				 			|| PhysicsSystem::collides(registry.motions.get(registry.players.entities[0]), registry.motions.get(registry.zombies.entities[i])))
			{
				Entity zombie = registry.zombies.entities[i];
				if (registry.zombies.has(zombie))
				{
					auto &zombie_comp = registry.zombies.get(zombie);
					zombie_comp.health -= registry.attacks.get(registry.players.entities[0]).damage;
					std::cout << "wow u r attacking so nice cool cool" << std::endl;

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

					}
					
					// Increase the experience of the player.
					if (registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage <= 1.0) {
						registry.screenStates.get(registry.screenStates.entities[0]).exp_percentage += registry.attacks.get(registry.players.entities[0]).damage / PLAYER_HEALTH;
					}
				}
			}
		}
		Cooldown &cooldown = registry.cooldowns.emplace(player);
		cooldown.timer_ms = COOLDOWN_PLAYER_ATTACK;
	}
}

void WorldSystem::update_enemy_death_animations(float elapsed_ms) {
    // Process each entity with a death animation
    for (Entity entity : registry.deathAnimations.entities) {
        auto& death_anim = registry.deathAnimations.get(entity);
        
        // Update alpha
        death_anim.duration_ms -= elapsed_ms;
        death_anim.alpha = death_anim.duration_ms / 500.0f;  // Linear fade out
        
        // Update position with increased slide speed and distance
        if (registry.motions.has(entity)) {
            auto& motion = registry.motions.get(entity);
            float slide_speed = 300.0f;  // pixels per second
            
            // Calculate movement
            float step_seconds = elapsed_ms / 1000.0f;
            vec2 movement = death_anim.slide_direction * (slide_speed * step_seconds);
            
            // Apply movement
            motion.position += movement;
        }
        
        // Remove entity when animation is complete
        if (death_anim.duration_ms <= 0) {
            registry.remove_all_components_of(entity);
        }
    }
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		close_window();
		return;
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
		return;
	}

	// Manual wave generation with 'g'
	if (action == GLFW_PRESS && key == GLFW_KEY_G)
	{
		spawn_manager.generate_wave(renderer);
		return;
	}

  if (action == GLFW_PRESS && key == GLFW_KEY_T)
	{
		test_mode = !test_mode;
		spawn_manager.set_test_mode(test_mode);
		std::cout << "Game " << (test_mode ? "entered" : "exited") << " test mode" << std::endl;
		return;
	}
  
	Entity player = registry.players.entities[0];
	Motion& motion = registry.motions.get(player);

	// Kung: I had to research online to determine how to deal with input with multiple keys.
	// The source I used is https://discourse.glfw.org/t/press-multiple-keys/1207

	// Move left
	if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			motion.velocity.x = 0;
		// } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		// 	motion.velocity.x = PLAYER_MOVE_LEFT_SPEED;
		// 	motion.velocity.y = PLAYER_MOVE_UP_SPEED;
		// } if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		// 	motion.velocity.x = PLAYER_MOVE_LEFT_SPEED;
		// 	motion.velocity.y = PLAYER_MOVE_DOWN_SPEED;
		} else motion.velocity.x = PLAYER_MOVE_LEFT_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
		motion.velocity.x = 0;
	}
	// Move right
	if (action == GLFW_PRESS && key == GLFW_KEY_D) {
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			motion.velocity.x = 0;
		} else motion.velocity.x = PLAYER_MOVE_RIGHT_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_D) {
		motion.velocity.x = 0;
	}

	// Move down
	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			motion.velocity.y = 0;
		} else motion.velocity.y = PLAYER_MOVE_DOWN_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_S) {
		motion.velocity.y = 0;
	}
	// Move up
	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			motion.velocity.y = 0;
		} else motion.velocity.y = PLAYER_MOVE_UP_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_W) {	
		motion.velocity.y = 0;
	}
  
  // State
  if (key == GLFW_KEY_A || key == GLFW_KEY_D || key == GLFW_KEY_S || key == GLFW_KEY_W) {
    State& state = registry.states.get(player);
    if (motion.velocity == vec2(0, 0)) {
      StateSystem::update_state(STATE::IDLE);
    }
    else {
      StateSystem::update_state(STATE::MOVE);
    }
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
	if (mouse_pos_x < motion.position.x && motion.scale.x > 0)
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
	if (mouse_pos_x > motion.position.x && motion.scale.x < 0)
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
	// on button press
	if (action == GLFW_PRESS)
	{

		int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
		int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

		// std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		// std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
	}

	if(action == GLFW_RELEASE && action == GLFW_MOUSE_BUTTON_LEFT) {
		player_attack();
	}
}

void WorldSystem::game_over()
{
	std::cout << "Game Over!" << std::endl;
	game_is_over = true;
	registry.screenStates.get(registry.screenStates.entities[0]).game_over = true;
}
