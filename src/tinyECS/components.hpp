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
	int pose = 0;
	int transition_ms;
	int pose_count;
	const TEXTURE_ASSET_ID* textures;
	bool loop = true;
	bool lock = false;
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

struct ScorchedEarth {
	
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
	float game_over_darken = -1;
	float game_over_counter_ms = 0;
	float hp_percentage = 1.0;
	float exp_percentage = 0.0;
	bool game_over = false;
	float lerp_timer = 0.0;
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

// Animation related components
struct DeathAnimation {
    vec2 slide_direction;  // Direction to slide
    float alpha = 1.0f;    // Transparency (1.0 = solid, 0.0 = invisible)
    float duration_ms;     // How long the animation lasts
};

struct HitEffect {
    float duration_ms = 200.0f;    // How long the hit effect lasts
    bool is_white = true;          // For white flash effect
};
