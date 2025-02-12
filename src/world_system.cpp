// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem() :
	points(0),
	max_towers(MAX_TOWERS_START),
	next_zombie_spawn(0),
	zombie_spawn_rate_ms(ZOMBIE_SPAWN_RATE_MS)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
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
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
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
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Farmer Defense: The Last Days", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());

	if (background_music == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str());
		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem* renderer_arg) {

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	//spawn new zombies
	next_zombie_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_zombie_spawn < 0.f) {
		// reset timer
		next_zombie_spawn = (ZOMBIE_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ZOMBIE_SPAWN_RATE_MS / 2);

		// create zombie with random initial position
		createZombie(renderer, vec2(50.f + uniform_dist(rng) * (WINDOW_WIDTH_PX - 100.f), 100.f));
	}

	//game over fade out
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {

		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		/* for A1, let the user press "R" to restart instead
		// restart the game once the death timer expires
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
		*/
	}

	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	max_towers = MAX_TOWERS_START;
	next_zombie_spawn = 0;
	zombie_spawn_rate_ms = ZOMBIE_SPAWN_RATE_MS;

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	// create grid lines
	int grid_line_width = GRID_LINE_WIDTH_PX;

	// create grid lines if they do not already exist
	if (grid_lines.size() == 0) {
		// vertical lines
		int cell_width = GRID_CELL_WIDTH_PX;
		for (int col = 0; col < 14 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
		}

		// horizontal lines
		int cell_height = GRID_CELL_HEIGHT_PX;
		for (int col = 0; col < 10 + 1; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(0, col * cell_height), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
		}
	}

	//spawn player in the middle of the screen
	createPlayer(renderer, vec2{WINDOW_WIDTH_PX/2, WINDOW_HEIGHT_PX/2});
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Loop over all collisions detected by the physics system
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// TODO A1: handle collision between deadly (projectile) and zombie
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// TODO A1: handle collision between tower and zombie
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		close_window();
		return;
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
		return;
	}

	Entity player = registry.players.entities[0];
	Motion& motion = registry.motions.get(player);
	// Move up
	if (action == GLFW_PRESS && key == GLFW_KEY_W) {
		motion.velocity.y = PLAYER_MOVE_UP_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_W) {
		motion.velocity.y = 0;
	}

	// Move left
	if (action == GLFW_PRESS && key == GLFW_KEY_A) {
		motion.velocity.x = PLAYER_MOVE_LEFT_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
		motion.velocity.x = 0;
	}

	// Move down
	if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		motion.velocity.y = PLAYER_MOVE_DOWN_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_S) {
		motion.velocity.y = 0;
	}

	// Move right
	if (action == GLFW_PRESS && key == GLFW_KEY_D) {
		motion.velocity.x = PLAYER_MOVE_RIGHT_SPEED;
	} else if (action == GLFW_RELEASE && key == GLFW_KEY_D) {
		motion.velocity.x = 0;
	}

	// Debugging (B) - not yet implemented!!!
	if (key == GLFW_KEY_B) {
		if (action == GLFW_RELEASE) {
			if (debugging.in_debug_mode) {
				debugging.in_debug_mode = false;
			}
			else {
				debugging.in_debug_mode = true;
			}
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;

	//change player facing direction
	Entity player = registry.players.entities[0];
	Motion& motion = registry.motions.get(player);

	//face right
	if (mouse_pos_x > motion.position.x && motion.scale.x < 0) {
		motion.scale.x = -motion.scale.x;
	}
	//face left
	if (mouse_pos_x < motion.position.x && motion.scale.x > 0) {
		motion.scale.x = -motion.scale.x;
	}
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// on button press
	if (action == GLFW_PRESS) {

		int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
		int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;

		

	}
}