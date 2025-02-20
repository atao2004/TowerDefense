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
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};

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
	PLAYER_IDLE = ZOMBIE_WALK_2 + 1,
	PLAYER_WALK_1 = PLAYER_IDLE + 1,
	PLAYER_WALK_2 = PLAYER_WALK_1 + 1,
	PLAYER_ACTION_1 = PLAYER_WALK_2 + 1,
	PLAYER_ACTION_2 = PLAYER_ACTION_1 + 1,
	GRASS = PLAYER_ACTION_2 + 1,
	SCORCHED_EARTH = GRASS + 1,
	TOOLBAR = SCORCHED_EARTH + 1,
	PAUSE = TOOLBAR + 1,
	TEXTURE_COUNT = PAUSE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	EGG = 0,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	UI = TEXTURED + 1,
	ZOMBIE = UI + 1,
	EFFECT_COUNT = ZOMBIE + 1
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

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

//
// game constants
//
const int WINDOW_WIDTH_PX = 1920;
const int WINDOW_HEIGHT_PX = 1080;

// Kung: Separate consts for testing on my Mac. Use these Anna!!
// const int WINDOW_WIDTH_PX = 1920 * 2 / 3;
// const int WINDOW_HEIGHT_PX = 1080 * 2 / 3;

const int GRID_CELL_WIDTH_PX = 60;
const int GRID_CELL_HEIGHT_PX = 60;
const int GRID_LINE_WIDTH_PX = 2;
const vec3 GRID_COLOR = vec3(0, 0, 0);

const int GRASS_DIMENSION_PX = 512;
const int DIRT_DIMENSION_PX = GRASS_DIMENSION_PX / 2;
const int SCORCHED_EARTH_BOUNDARY = DIRT_DIMENSION_PX / 4;

const int MAX_ZOMBIES = 5;

const int PLAYER_HEALTH = 100;
const int ZOMBIE_HEALTH = 20;
const int ZOMBIE_SPAWN_RATE_MS = 2 * 1000;

const int PROJECTILE_DAMAGE = 10;

//control player movement
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

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// textures
const float PLAYER_WIDTH = 61;
const float PLAYER_HEIGHT = 90;
const float ZOMBIE_WIDTH = 61;
const float ZOMBIE_HEIGHT = 90;

// cooldown
const int COOLDOWN_ENEMY_ATTACK = 1000;
const int COOLDOWN_PLAYER_ATTACK = 1000;

// animation
const int PLAYER_MOVE_FRAME_DELAY = 500;
const int PLAYER_ATTACK_FRAME_DELAY = 250;
const int ZOMBIE_MOVE_FRAME_DELAY = 500;

const TEXTURE_ASSET_ID PLAYER_ANIMATION_MOVE[] = {
	TEXTURE_ASSET_ID::PLAYER_WALK_1,
	TEXTURE_ASSET_ID::PLAYER_WALK_2
};

const TEXTURE_ASSET_ID PLAYER_ANIMATION_ATTACK[] = {
	TEXTURE_ASSET_ID::PLAYER_ACTION_1,
	TEXTURE_ASSET_ID::PLAYER_ACTION_2
};

const TEXTURE_ASSET_ID ZOMBIE_ANIMATION[] = {
	TEXTURE_ASSET_ID::ZOMBIE_WALK_1,
	TEXTURE_ASSET_ID::ZOMBIE_WALK_2
};

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
