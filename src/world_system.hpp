	#pragma once

	// internal
	#include "common.hpp"
	#include "spawn_manager.hpp"

	// stlib
	#include <vector>
	#include <random>

	#define SDL_MAIN_HANDLED
	#include <SDL.h>
	#include <SDL_mixer.h>

	#include "render_system.hpp"

	// Container for all our entities and game logic.
	// Individual rendering / updates are deferred to the update() methods.
	class WorldSystem
	{
	public:
		WorldSystem();

	// creates main window
	GLFWwindow *create_window();

	// starts and loads music and sound effects
	bool start_and_load_sounds();

	// call to close the window
	void close_window();
	
	static void start_cg(RenderSystem* renderer);
	bool detectButtons();
	void clearButtons();
	void levelUpHelper(std::set<int> unique_numbers, bool buttonsCreated);

	// starts the game
	void init(RenderSystem *renderer);
	void restart_splash_screen();

	// releases all associated resources
	~WorldSystem();

	// Returns the game_screen (inspired by Assignment #2)
	static GAME_SCREEN_ID get_game_screen()
	{
		return game_screen;
	}

	static void set_game_screen(GAME_SCREEN_ID gs)
	{
		game_screen = gs;
	}

	// steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// should the game be over ?
	bool is_over() const;

	void game_over();
	static bool game_is_over;
	unsigned int current_seed; // current type of seed the player has selected
	// Current experience level of the player
	unsigned int level;
	// Number of players stopped by the towers, displayed in the window title
	unsigned int points;

	// increases player experience
	static void increase_exp();
	void increase_level();

	static int get_current_day() { return current_day; }

	static bool player_is_dashing;

	void increment_points();

private:
	static int current_day;

	float mouse_pos_x = 0.0f;
	float mouse_pos_y = 0.0f;

	void player_attack();
	void update_enemy_death_animations(float elapsed_ms);
	void update_screen_shake(float elapsed_ms);

	// Kung: player movement helper function
	void player_movement(int key, int action, Motion &player_motion);
	void player_movement_tutorial(int key, int action, Motion &player_motion);

	// input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button_pressed(int button, int action, int mods);

	// restart the level or the tutorial
	void restart_common_tasks(vec2 map_dimensions);
	void restart_overlay_renders(vec2 player_pos);
	void restart_game();
	void restart_tutorial();
	void create_tutorial_enemies();
	void check_tutorial_enemies();

	// OpenGL window handle
	GLFWwindow *window;

	// game_screen (inspired by Assignment #2)
	static GAME_SCREEN_ID game_screen;

	// Game state
	RenderSystem *renderer;
	float current_speed;

	// grid
	std::vector<Entity> grid_lines;

	// music references
	Mix_Music *current_bgm; // handle switching soundtrack
	Mix_Music *night_bgm;
	Mix_Music *day_bgm;
	Mix_Music *combat_bgm;
	Mix_Chunk *sword_attack_sound;
	Mix_Chunk *running_on_grass_sound;
	static Mix_Chunk *game_over_sound;

	// Manage spawning zombies
	SpawnManager spawn_manager;

	bool test_mode = false;
	bool tutorial_mode = true;

	// Sound effects
	float movement_sound_timer = 0.f;
	bool is_movement_sound_playing = false;
	void update_movement_sound(float elapsed_ms);

	void update_camera();

	bool chicken_summoned = false;

	float rest_timer_ms = 0.f;		  // Timer for rest period between days
	float enemy_spawn_timer_ms = 0.f; // Timer for spawning enemies
	bool day_in_progress = false;	  // Whether the current day's enemies are still spawning
	int enemies_spawned_today = 0;	  // Track how many enemies spawned in current day
	int enemies_to_spawn_today = 0;	  // Total enemies to spawn today

	void advance_to_next_day();
	int calculate_enemies_for_day(int day);
	void updateDayInProgress(float elapsed_ms_since_last_update);

	void loadGame();
	void saveGame();

	void plant_seed();
  
	// Player dash
	void update_dash(float elapsed_ms_since_last_update);
	
    float dash_timer_ms = 0.0f;
    float dash_cooldown_ms = 0.0f;
    vec2 dash_direction = {0.0f, 0.0f};
};
