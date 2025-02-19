#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

#ifdef Status
#undef Status
#endif

struct Attack
{
	int range;
	float damage = 10.0;
};

struct Death
{

};

struct Status {
    std::string type;      // Status type (e.g. "injured", "poisoned")
    float duration_ms;     // Counts down to 0
    float value;          // Effect value (damage, etc)
};

struct StatusComponent {
    std::vector<Status> active_statuses;
};

struct Dimension {
	int width;
	int height;
};

struct Experience
{
	int exp;
};

struct Cooldown {
	int timer_ms;
};

struct Motion
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
	Motion()
	{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
	}
};

struct Texture
{
};

struct Player
{
	float health;
};

struct Zombie
{
float health;
};

enum class STATE
{
	IDLE = 0,
	MOVE = 1,
	ATTACK = 2
};

struct State
{
	STATE state;
};

struct Animation
{
	float timer_ms = 0;
};

// Tower
struct Tower
{
	float range;  // for vision / detection
	int timer_ms; // when to shoot - this could also be a separate timer component...
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity &other) { this->other = other; };
};

struct Grass {

};

struct Toolbar {

};

struct Pause {

};

// Sets the brightness of the screen
// Includes HP and EXP parameters
struct ScreenState
{
	float darken_screen_factor = -1;
	float hp_percentage = 1.0;
	float exp_percentage = 0.0;
};

// used to hold grid line start and end positions
struct GridLine
{
	vec2 start_pos = {0, 0};
	vec2 end_pos = {10, 10}; // default to diagonal line
};

// Single Vertex Buffer element for non-textured meshes (chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct DeathAnimation {
    vec2 slide_direction;  // Direction to slide
    float alpha = 1.0f;    // Transparency (1.0 = solid, 0.0 = invisible)
    float duration_ms;     // How long the animation lasts
};

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
	INVADER = 0,
	TOWER = INVADER + 1,
	ZOMBIE_WALK_1 = TOWER + 1,
	ZOMBIE_WALK_2 = ZOMBIE_WALK_1 + 1,
	PLAYER_IDLE = ZOMBIE_WALK_2 + 1,
	PLAYER_WALK_1  = PLAYER_IDLE + 1,
	PLAYER_WALK_2  = PLAYER_WALK_1 + 1,
	PLAYER_ACTION_1  = PLAYER_WALK_2 + 1,
	PLAYER_ACTION_2  = PLAYER_ACTION_1 + 1,
	GRASS = PLAYER_ACTION_2 + 1,
	TOOLBAR = GRASS + 1,
	PAUSE = TOOLBAR + 1,
	TEXTURE_COUNT = PAUSE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	EGG = 0,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	VIGNETTE = TEXTURED + 1,
	ZOMBIE = VIGNETTE + 1,
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
