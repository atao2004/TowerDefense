#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>			   // vec2
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/vec3.hpp>			   // vec3
#include <glm/mat3x3.hpp>		   // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string &name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string &name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string &name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string &name) { return data_path() + "/meshes/" + std::string(name); };

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID
{
	TOWER = 0,
	SEED_1 = TOWER + 1,
	ZOMBIE_WALK_1 = SEED_1 + 1,
	ZOMBIE_WALK_2 = ZOMBIE_WALK_1 + 1,
	ZOMBIE_SPAWN_1 = ZOMBIE_WALK_2 + 1,
	ZOMBIE_SPAWN_2 = ZOMBIE_SPAWN_1 + 1,
	PLAYER_IDLE = ZOMBIE_SPAWN_2 + 1,
	PLAYER_WALK_1 = PLAYER_IDLE + 1,
	PLAYER_WALK_2 = PLAYER_WALK_1 + 1,
	PLAYER_ACTION_1 = PLAYER_WALK_2 + 1,
	PLAYER_ACTION_2 = PLAYER_ACTION_1 + 1,
	SCORCHED_EARTH = PLAYER_ACTION_2 + 1,
	TUTORIAL_MOVE = SCORCHED_EARTH + 1,
	TUTORIAL_MOVE_W = TUTORIAL_MOVE + 1,
	TUTORIAL_MOVE_A = TUTORIAL_MOVE_W + 1,
	TUTORIAL_MOVE_S = TUTORIAL_MOVE_A + 1,
	TUTORIAL_MOVE_D = TUTORIAL_MOVE_S + 1,
	TUTORIAL_ATTACK = TUTORIAL_MOVE_D + 1,
	TUTORIAL_ATTACK_ANIMATED = TUTORIAL_ATTACK + 1,
	TUTORIAL_PLANT = TUTORIAL_ATTACK_ANIMATED + 1,
	TUTORIAL_PLANT_ANIMATED = TUTORIAL_PLANT + 1,
	TUTORIAL_RESTART = TUTORIAL_PLANT_ANIMATED + 1,
	TUTORIAL_RESTART_ANIMATED = TUTORIAL_RESTART + 1,
	TUTORIAL_ARROW = TUTORIAL_RESTART_ANIMATED + 1,
	TOOLBAR = TUTORIAL_ARROW + 1,
	PAUSE = TOOLBAR + 1,
	PLAYER_ATTACK_SLASH_1 = PAUSE + 1,
	PLAYER_ATTACK_SLASH_2 = PLAYER_ATTACK_SLASH_1 + 1,
	PLAYER_ATTACK_SLASH_3 = PLAYER_ATTACK_SLASH_2 + 1,
	PLAYER_ATTACK_SLASH_4 = PLAYER_ATTACK_SLASH_3 + 1,
	PLAYER_ATTACK_SLASH_5 = PLAYER_ATTACK_SLASH_4 + 1,
	PLAYER_ATTACK_SLASH_6 = PLAYER_ATTACK_SLASH_5 + 1,
	PLAYER_ATTACK_SLASH_7 = PLAYER_ATTACK_SLASH_6 + 1,
	PLAYER_ATTACK_SLASH_8 = PLAYER_ATTACK_SLASH_7 + 1,
	PLAYER_ATTACK_SLASH_9 = PLAYER_ATTACK_SLASH_8 + 1,
	PROJECTILE = PLAYER_ATTACK_SLASH_9 + 1,
	GAMEOVER = PROJECTILE + 1,
	GRASS_BACKGROUND = GAMEOVER + 1,
	FLOWER_1 = GRASS_BACKGROUND + 1,
	STONE_1 = FLOWER_1 + 1,
	GRASS_DECORATION_1 = STONE_1 + 1,
	TREE_1 = GRASS_DECORATION_1 + 1,
	FARMLAND_1 = TREE_1 + 1,
	SKELETON_IDLE1 = FARMLAND_1 + 1,
	SKELETON_IDLE2 = SKELETON_IDLE1 + 1,
	SKELETON_IDLE3 = SKELETON_IDLE2 + 1,
	SKELETON_IDLE4 = SKELETON_IDLE3 + 1,
	SKELETON_IDLE5 = SKELETON_IDLE4 + 1,
	SKELETON_IDLE6 = SKELETON_IDLE5 + 1,
	SKELETON_WALK1 = SKELETON_IDLE6 + 1,
	SKELETON_WALK2 = SKELETON_WALK1 + 1,
	SKELETON_WALK3 = SKELETON_WALK2 + 1,
	SKELETON_WALK4 = SKELETON_WALK3 + 1,
	SKELETON_WALK5 = SKELETON_WALK4 + 1,
	SKELETON_WALK6 = SKELETON_WALK5 + 1,
	SKELETON_WALK7 = SKELETON_WALK6 + 1,
	SKELETON_WALK8 = SKELETON_WALK7 + 1,
	SKELETON_ATTACK1 = SKELETON_WALK8 + 1,
	SKELETON_ATTACK2 = SKELETON_ATTACK1 + 1,
	SKELETON_ATTACK3 = SKELETON_ATTACK2 + 1,
	SKELETON_ATTACK4 = SKELETON_ATTACK3 + 1,
	SKELETON_ATTACK5 = SKELETON_ATTACK4 + 1,
	SKELETON_ATTACK6 = SKELETON_ATTACK5 + 1,
	SKELETON_ATTACK7 = SKELETON_ATTACK6 + 1,
	SKELETON_ATTACK8 = SKELETON_ATTACK7 + 1,
	SKELETON_ATTACK9 = SKELETON_ATTACK8 + 1,
	ARROW = SKELETON_ATTACK9 + 1,
	ORC_WALK1 = ARROW + 1,
	ORC_WALK2 = ORC_WALK1 + 1,
	ORC_WALK3 = ORC_WALK2 + 1,
	ORC_WALK4 = ORC_WALK3 + 1,
	ORC_WALK5 = ORC_WALK4 + 1,
	ORC_WALK6 = ORC_WALK5 + 1,
	ORC_WALK7 = ORC_WALK6 + 1,
	ORC_WALK8 = ORC_WALK7 + 1,

	PLAYER_IDLE1 = ORC_WALK8 + 1,
	PLAYER_IDLE2 = PLAYER_IDLE1 + 1,
	PLAYER_IDLE3 = PLAYER_IDLE2 + 1,
	PLAYER_IDLE4 = PLAYER_IDLE3 + 1,
	PLAYER_IDLE5 = PLAYER_IDLE4 + 1,
	PLAYER_IDLE6 = PLAYER_IDLE5 + 1,

	PLAYER_WALK1 = PLAYER_IDLE6 + 1,
	PLAYER_WALK2 = PLAYER_WALK1 + 1,
	PLAYER_WALK3 = PLAYER_WALK2 + 1,
	PLAYER_WALK4 = PLAYER_WALK3 + 1,
	PLAYER_WALK5 = PLAYER_WALK4 + 1,
	PLAYER_WALK6 = PLAYER_WALK5 + 1,
	PLAYER_WALK7 = PLAYER_WALK6 + 1,
	PLAYER_WALK8 = PLAYER_WALK7 + 1,

	PLAYER_ATTACK_ACTION1 = PLAYER_WALK8 + 1,
	PLAYER_ATTACK_ACTION2 = PLAYER_ATTACK_ACTION1 + 1,
	PLAYER_ATTACK_ACTION3 = PLAYER_ATTACK_ACTION2 + 1,
	PLAYER_ATTACK_ACTION4 = PLAYER_ATTACK_ACTION3 + 1,
	PLAYER_ATTACK_ACTION5 = PLAYER_ATTACK_ACTION4 + 1,
	PLAYER_ATTACK_ACTION6 = PLAYER_ATTACK_ACTION5 + 1,

	PLANT_2_IDLE_F = PLAYER_ATTACK_ACTION6 + 1,
	PLANT_2_IDLE_B = PLANT_2_IDLE_F + 1,
	PLANT_2_IDLE_S = PLANT_2_IDLE_B + 1,

	PLANT_2_ATTACK_F1 = PLANT_2_IDLE_S + 1,
	PLANT_2_ATTACK_F2 = PLANT_2_ATTACK_F1 + 1,
	PLANT_2_ATTACK_B1 = PLANT_2_ATTACK_F2 + 1,
	PLANT_2_ATTACK_B2 = PLANT_2_ATTACK_B1 + 1,
	PLANT_2_ATTACK_S1 = PLANT_2_ATTACK_B2 + 1,
	PLANT_2_ATTACK_S2 = PLANT_2_ATTACK_S1 + 1,
	TEXTURE_COUNT = PLANT_2_ATTACK_S2 + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	EGG = 0,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	UI = TEXTURED + 1,
	VIGNETTE = UI + 1,
	ZOMBIE = VIGNETTE + 1,
	PLAYER = ZOMBIE + 1,
	EFFECT_COUNT = PLAYER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

// This comes from Assignment #2 and has been adapted for our needs.
enum class GAME_SCREEN_ID
{
	PLAYING = 0,
	TUTORIAL = PLAYING + 1,
	TEST = TUTORIAL + 1,
	GAME_SCREEN_COUNT = TEST + 1
};
const int game_screen_count = (int)GAME_SCREEN_ID::GAME_SCREEN_COUNT;

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

//
// game constants
//

inline int WINDOW_WIDTH_PX = 1920;
inline int WINDOW_HEIGHT_PX = 1080;

const int GRID_CELL_WIDTH_PX = 60;
const int GRID_CELL_HEIGHT_PX = 60;
const int GRID_LINE_WIDTH_PX = 2;
const vec3 GRID_COLOR = vec3(0, 0, 0);

inline int MAP_WIDTH_TILE_NUM = 40;
inline int MAP_HEIGHT_TILE_NUM = 30;
inline int MAP_WIDTH_PX = GRID_CELL_WIDTH_PX * MAP_WIDTH_TILE_NUM;
inline int MAP_HEIGHT_PX = GRID_CELL_HEIGHT_PX * MAP_HEIGHT_TILE_NUM;

inline int TUTORIAL_WIDTH_TILE_NUM = 40;
inline int TUTORIAL_HEIGHT_TILE_NUM = 4;
inline int TUTORIAL_WIDTH_PX = GRID_CELL_WIDTH_PX * TUTORIAL_WIDTH_TILE_NUM;
inline int TUTORIAL_HEIGHT_PX = GRID_CELL_HEIGHT_PX * TUTORIAL_HEIGHT_TILE_NUM;

const int SCORCHED_EARTH_DIMENSION_PX = 256;

const int PLAYER_HEALTH = 100;
const int ZOMBIE_HEALTH = 20;
const int ZOMBIE_DAMAGE = 5;
const float BASE_ENEMY_SPEED = 500.0f; // Base movement speed for enemies
const int ZOMBIE_SPAWN_RATE_MS = 2 * 1000;

// Skeleton properties
const float SKELETON_WIDTH = 200.f;
const float SKELETON_HEIGHT = 200.f;
const float SKELETON_HEALTH = 75.f;
const float SKELETON_SPEED = 75.f;
const float SKELETON_FRAME_DELAY = 500.f;
const float SKELETON_ARROW_DAMAGE = 15.f;

const TEXTURE_ASSET_ID SKELETON_IDLE_ANIMATION[] = {
	TEXTURE_ASSET_ID::SKELETON_IDLE1,
	TEXTURE_ASSET_ID::SKELETON_IDLE2,
	TEXTURE_ASSET_ID::SKELETON_IDLE3,
	TEXTURE_ASSET_ID::SKELETON_IDLE4,
	TEXTURE_ASSET_ID::SKELETON_IDLE5,
	TEXTURE_ASSET_ID::SKELETON_IDLE6};
const int SKELETON_IDLE_FRAMES = 6;

const TEXTURE_ASSET_ID SKELETON_WALK_ANIMATION[] = {
	TEXTURE_ASSET_ID::SKELETON_WALK1,
	TEXTURE_ASSET_ID::SKELETON_WALK2,
	TEXTURE_ASSET_ID::SKELETON_WALK3,
	TEXTURE_ASSET_ID::SKELETON_WALK4,
	TEXTURE_ASSET_ID::SKELETON_WALK5,
	TEXTURE_ASSET_ID::SKELETON_WALK6,
	TEXTURE_ASSET_ID::SKELETON_WALK7,
	TEXTURE_ASSET_ID::SKELETON_WALK8};

const TEXTURE_ASSET_ID SKELETON_ATTACK_ANIMATION[] = {
	TEXTURE_ASSET_ID::SKELETON_ATTACK1,
	TEXTURE_ASSET_ID::SKELETON_ATTACK2,
	TEXTURE_ASSET_ID::SKELETON_ATTACK3,
	TEXTURE_ASSET_ID::SKELETON_ATTACK4,
	TEXTURE_ASSET_ID::SKELETON_ATTACK5,
	TEXTURE_ASSET_ID::SKELETON_ATTACK6,
	TEXTURE_ASSET_ID::SKELETON_ATTACK7,
	TEXTURE_ASSET_ID::SKELETON_ATTACK8,
	TEXTURE_ASSET_ID::SKELETON_ATTACK9};

const TEXTURE_ASSET_ID PLANT_2_IDLE_ANIMATION[] = {
	TEXTURE_ASSET_ID::PLANT_2_IDLE_F,
	TEXTURE_ASSET_ID::PLANT_2_IDLE_B,
	TEXTURE_ASSET_ID::PLANT_2_IDLE_S};

const TEXTURE_ASSET_ID PLANT_2_ATTACK_ANIMATION[] = {
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_F1,
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_F2,
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_B1,
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_B2,
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_S1,
	TEXTURE_ASSET_ID::PLANT_2_ATTACK_S2

};

// Animation frame counts
const int SKELETON_WALK_FRAMES = 8;
const int SKELETON_ATTACK_FRAMES = 9;

// Animation durations
const int SKELETON_IDLE_DURATION = 800;
const int SKELETON_WALK_DURATION = 800;
const int SKELETON_ATTACK_DURATION = 1800;

const int PROJECTILE_DAMAGE = 10;

// control player movement
const int PLAYER_MOVE_UP_SPEED = -150;
const int PLAYER_MOVE_DOWN_SPEED = -PLAYER_MOVE_UP_SPEED;
const int PLAYER_MOVE_LEFT_SPEED = -150;
const int PLAYER_MOVE_RIGHT_SPEED = -PLAYER_MOVE_LEFT_SPEED;

// These are hard coded to the dimensions of the entity's texture

// players are 64x64 px, but cells are 60x60
const float PLAYER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float PLAYER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// towers are 64x64 px, but cells are 60x60
const float TOWER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float TOWER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// Add these camera constants
const float CAMERA_VIEW_WIDTH = WINDOW_WIDTH_PX * 4 / 5;
const float CAMERA_VIEW_HEIGHT = WINDOW_HEIGHT_PX * 4 / 5;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// textures
const float PLAYER_WIDTH = 50;
const float PLAYER_HEIGHT = 50;
const float ZOMBIE_WIDTH = 50;
const float ZOMBIE_HEIGHT = 50;
const float SLASH_WIDTH = 60;
const float SLASH_HEIGHT = 60;

// cooldown
const int COOLDOWN_ENEMY_ATTACK = 1000;
const int COOLDOWN_PLAYER_ATTACK = 400;
const int COOLDOWN_ZOMBIE_SPAWN = 3000;

// player movement boundaries
const int PLAYER_LEFT_BOUNDARY = 0;
const int PLAYER_RIGHT_BOUNDARY = MAP_WIDTH_PX - GRID_CELL_WIDTH_PX;
const int PLAYER_UP_BOUNDARY = 0;
const int PLAYER_DOWN_BOUNDARY = MAP_HEIGHT_PX - GRID_CELL_HEIGHT_PX;

const int PLAYER_RIGHT_BOUNDARY_TUTORIAL = TUTORIAL_WIDTH_PX - GRID_CELL_WIDTH_PX;
const int PLAYER_DOWN_BOUNDARY_TUTORIAL = TUTORIAL_HEIGHT_PX - GRID_CELL_HEIGHT_PX;

// animation (texture)
const TEXTURE_ASSET_ID PLAYER_IDLE_ANIMATION[] = {
	TEXTURE_ASSET_ID::PLAYER_IDLE1,
	TEXTURE_ASSET_ID::PLAYER_IDLE2,
	TEXTURE_ASSET_ID::PLAYER_IDLE3,
	TEXTURE_ASSET_ID::PLAYER_IDLE4,
	TEXTURE_ASSET_ID::PLAYER_IDLE5,
	TEXTURE_ASSET_ID::PLAYER_IDLE6};

const TEXTURE_ASSET_ID PLAYER_MOVE_ANIMATION[] = {
	// TEXTURE_ASSET_ID::PLAYER_WALK_1,
	// TEXTURE_ASSET_ID::PLAYER_WALK_2
	TEXTURE_ASSET_ID::PLAYER_WALK1,
	TEXTURE_ASSET_ID::PLAYER_WALK2,
	TEXTURE_ASSET_ID::PLAYER_WALK3,
	TEXTURE_ASSET_ID::PLAYER_WALK4,
	TEXTURE_ASSET_ID::PLAYER_WALK5,
	TEXTURE_ASSET_ID::PLAYER_WALK6,
	TEXTURE_ASSET_ID::PLAYER_WALK7,
	TEXTURE_ASSET_ID::PLAYER_WALK8};
const TEXTURE_ASSET_ID PLAYER_ATTACK_ANIMATION[] = {
	// TEXTURE_ASSET_ID::PLAYER_ACTION_1,
	// TEXTURE_ASSET_ID::PLAYER_ACTION_2
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION1,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION2,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION3,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION4,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION5,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_ACTION6};
const TEXTURE_ASSET_ID ZOMBIE_MOVE_ANIMATION[] = {
	// TEXTURE_ASSET_ID::ZOMBIE_WALK_1,
	// TEXTURE_ASSET_ID::ZOMBIE_WALK_2
	TEXTURE_ASSET_ID::ORC_WALK1,
	TEXTURE_ASSET_ID::ORC_WALK2,
	TEXTURE_ASSET_ID::ORC_WALK3,
	TEXTURE_ASSET_ID::ORC_WALK4,
	TEXTURE_ASSET_ID::ORC_WALK5,
	TEXTURE_ASSET_ID::ORC_WALK6,
	TEXTURE_ASSET_ID::ORC_WALK7,
	TEXTURE_ASSET_ID::ORC_WALK8};
const TEXTURE_ASSET_ID ZOMBIE_SPAWN_ANIMATION[] = {
	TEXTURE_ASSET_ID::ZOMBIE_SPAWN_1,
	TEXTURE_ASSET_ID::ZOMBIE_SPAWN_2};
const TEXTURE_ASSET_ID SLASH_ANIMATION[] = {
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_1,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_2,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_3,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_4,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_5,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_6,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_7,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_8,
	TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_9,
};

// animation (size)
const int PLAYER_IDLE_SIZE = sizeof(PLAYER_IDLE_ANIMATION) / sizeof(PLAYER_IDLE_ANIMATION[0]);
const int PLAYER_MOVE_SIZE = sizeof(PLAYER_MOVE_ANIMATION) / sizeof(PLAYER_MOVE_ANIMATION[0]);
const int PLAYER_ATTACK_SIZE = sizeof(PLAYER_ATTACK_ANIMATION) / sizeof(PLAYER_ATTACK_ANIMATION[0]);
const int ZOMBIE_MOVE_SIZE = sizeof(ZOMBIE_MOVE_ANIMATION) / sizeof(ZOMBIE_MOVE_ANIMATION[0]);
const int ZOMBIE_SPAWN_SIZE = sizeof(ZOMBIE_SPAWN_ANIMATION) / sizeof(ZOMBIE_SPAWN_ANIMATION[0]);
const int SLASH_SIZE = sizeof(SLASH_ANIMATION) / sizeof(SLASH_ANIMATION[0]);

const TEXTURE_ASSET_ID DECORATION_LIST[] = {
	TEXTURE_ASSET_ID::TEXTURE_COUNT, // 0 is not used
	TEXTURE_ASSET_ID::GRASS_BACKGROUND,
	TEXTURE_ASSET_ID::FLOWER_1,
	TEXTURE_ASSET_ID::STONE_1,
	TEXTURE_ASSET_ID::GRASS_DECORATION_1,
	TEXTURE_ASSET_ID::TREE_1,
	TEXTURE_ASSET_ID::FARMLAND_1,
};

const vec2 DECORATION_SIZE_LIST[] = {
	vec2(0, 0),
	vec2(60, 60),
	vec2(6 * 2, 6 * 2), // scale the original size by a factor to make it bigger
	vec2(11 * 2, 8 * 2),
	vec2(9 * 2, 6 * 2),
	vec2(60 * 3, 60 * 3),
	vec2(60, 60),
};

// animation (time)
const int PLAYER_IDLE_DURATION = 1000;
const int PLAYER_MOVE_DURATION = 1000;
const int PLAYER_ATTACK_DURATION = 400;
const int ZOMBIE_MOVE_DURATION = 1000;
const int ZOMBIE_SPAWN_DURATION = 3000;
const int SLASH_DURATION = PLAYER_ATTACK_DURATION;

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform
{
	mat3 mat = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
