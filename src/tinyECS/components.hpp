#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "plants.hpp"

#ifdef Status
#undef Status
#endif
#include "../ext/json.hpp"
using json = nlohmann::json;

struct Attack
{
    int range;
    float damage = 10.0;

    json toJSON() const
    {
        return json{
            {"range", range},
            {"damage", damage}};
    }
};

struct Death
{
    json toJSON() const
    {
        return json{};
    };
};

struct Inventory
{
    int seedCount[NUM_SEED_TYPES]; // count of seeds indexed by their type, could also use map for this
    int seedPosition[8];
    int seedAtToolbar[8];
    json toJSON() const
    {
        nlohmann::json seedJson;
        nlohmann::json seedPositionJson;
        nlohmann::json seedAtToolbarJson;
        for (int i = 0; i < NUM_SEED_TYPES; i++)
            seedJson[std::to_string(i)] = seedCount[i];
        for (int i = 0; i < 8; i++)
            seedPositionJson[std::to_string(i)] = seedPosition[i];
        for (int i = 0; i < 8; i++)
            seedAtToolbarJson[std::to_string(i)] = seedAtToolbar[i];
        return json{
            {"seedCount", seedJson},
            {"seedPosition", seedPositionJson},
            {"seedAtToolbar", seedAtToolbarJson}
        };
    }
};

struct Status
{
    std::string type;  // Status type (e.g. "injured", "poisoned")
    float duration_ms; // Counts down to 0
    float value;       // Effect value (damage, etc)

    json toJSON() const
    {
        return json{
            {"type", type},
            {"duration_ms", duration_ms},
            {"value", value}};
    }
};

struct StatusComponent
{
    std::vector<Status> active_statuses;
    json toJSON() const
    {
        json statusesJson = json::array();
        for (const auto &status : active_statuses)
        {
            statusesJson.push_back(status.toJSON());
        }
        return json{{"active_statuses", statusesJson}};
    }
};

struct Dimension
{
    int width;
    int height;

    json toJSON() const
    {
        return json{
            {"width", width},
            {"height", height}};
    }
};

struct Experience
{
    int exp;

    json toJSON() const
    {
        return json{
            {"exp", exp}};
    }
};

struct Cooldown
{
    int timer_ms;

    json toJSON() const
    {
        return json{
            {"timer_ms", timer_ms}};
    }
};

struct Motion
{
    vec2 position = {0, 0};
    float angle = 0;
    vec2 velocity = {0, 0};
    vec2 scale = {10, 10};

    json toJSON() const
    {
        return json{
            {"position", {position.x, position.y}},
            {"angle", angle},
            {"velocity", {velocity.x, velocity.y}},
            {"scale", {scale.x, scale.y}}};
    }
};

struct VisualScale
{
    vec2 scale = {1.0f, 1.0f}; // Default is no scaling
    json toJSON() const
    {
        return json{
            {"scale", {scale.x, scale.y}}};
    }
};

struct Texture
{
    json toJSON() const
    {
        return json{};
    };
};

struct Player
{
    float health;
    float health_max;
    json toJSON() const
    {
        return json{
            {"health", health},
            {"health_max", health_max}};
    }
};

struct Zombie
{
    float health;
    json toJSON() const
    {
        return json{
            {"health", health}};
    }
};

struct Enemy
{
    float health;
    float speed;
    json toJSON() const
    {
        return json{
            {"health", health},
            {"speed", speed}};
    }
};

// Skeleton enemy component
struct Skeleton
{
    float attack_range = 400.f;         // Attack range
    float stop_distance = 200.f;        // Distance to stop moving
    float attack_cooldown_ms = 10000.f; // Attack cooldown time
    float cooldown_timer_ms = 0.f;      // Current cooldown timer
    Entity target = {};                 // Current target
    bool is_attacking = false;          // Is currently attacking
    float health = SKELETON_HEALTH;     // Health of the skeleton

    float attack_timer_ms = 0.f; // Timer for when to fire arrow during attack
    bool arrow_fired = false;    // Whether the arrow was fired for current attack

    enum class State
    {
        IDLE,
        WALK,
        ATTACK
    };

    State current_state = State::IDLE;
    json toJSON() const
    {
        return json{
            {"attack_range", attack_range},
            {"stop_distance", stop_distance},
            {"attack_cooldown_ms", attack_cooldown_ms},
            {"cooldown_timer_ms", cooldown_timer_ms},
            {"target", target.id()},
            {"is_attacking", is_attacking},
            {"health", health},
            {"attack_timer_ms", attack_timer_ms},
            {"arrow_fired", arrow_fired},
            {"current_state", static_cast<int>(current_state)} // Enum to int
        };
    }
};

struct OrcRider
{
    enum class State
    {
        IDLE,
        WALK,
        HUNT
    };

    State current_state = State::IDLE;
    Entity target = {};

    float detection_range = 100000.0f; // Range to start walking towards player
    float hunt_range = 500.0f;        // Range to start hunting behavior
    float charge_speed = 400.0f;      // Speed during charge
    float walk_speed = 150.0f;        // Speed when walking
    float charge_distance = 400.0f;   // How far to charge
    int damage = 20;                  // Damage on successful charge hit

    // Hunting control variables
    bool is_hunting = false;
    bool is_charging = false;
    float hunt_timer_ms = 0.0f;           // Timer for the hunt animation
    float charge_timer_ms = 0.0f;         // Timer for the charge
    vec2 charge_direction = {0.0f, 0.0f}; // Direction of charge

    // For collision detection during charge
    bool has_hit_player = false;

    json toJSON() const
    {
        return json{
            {"current_state", static_cast<int>(current_state)},
            {"target", target.id()},
            {"detection_range", detection_range},
            {"hunt_range", hunt_range},
            {"charge_speed", charge_speed},
            {"walk_speed", walk_speed},
            {"charge_distance", charge_distance},
            {"damage", damage},
            {"is_hunting", is_hunting},
            {"is_charging", is_charging},
            {"hunt_timer_ms", hunt_timer_ms},
            {"charge_timer_ms", charge_timer_ms},
            {"charge_direction", {charge_direction.x, charge_direction.y}},
            {"has_hit_player", has_hit_player}};
    }
};

struct Arrow
{
    Entity source = {};          // Source entity that fired the arrow
    float damage = 15.f;         // Damage value
    float lifetime_ms = 2000.f;  // Lifetime in milliseconds
    float speed = 250.f;         // Flight speed
    vec2 direction = {0.f, 0.f}; // Flight direction
    json toJSON() const
    {
        return json{
            {"source", source.id()},
            {"damage", damage},
            {"lifetime_ms", lifetime_ms},
            {"speed", speed},
            {"direction", {direction.x, direction.y}}};
    }
};

struct ZombieSpawn
{
    json toJSON() const
    {
        return json{}; // Empty, as it doesn't hold data
    }
};

struct Projectile
{
    Entity source = {};          // The tower that fired this projectile
    float damage = 10.f;         // Damage taken from tower
    float speed = 200.f;         // Projectile speed
    float lifetime_ms = 2000.f;  // How long the projectile lasts
    vec2 direction = {0.f, 0.f}; // Direction of the projectile
    bool invincible = false;     // Projectile is not destroyed upon collision
    json toJSON() const
    {
        return json{
            {"source", source.id()},
            {"damage", damage},
            {"speed", speed},
            {"lifetime_ms", lifetime_ms},
            {"direction", {direction.x, direction.y}},
            {"invincible", invincible}};
    }
};

// For Milestone #2.
struct Seed
{
    int type; // Maybe make it a string? or in my opinion maybe an enum would be better
    float timer;

    json toJSON() const
    {
        return json{
            {"type", type},
            {"timer", timer}};
    }
};

enum class STATE
{
    IDLE = 0,
    MOVE = 1,
    ATTACK = 2,
    STATE_COUNT = ATTACK + 1
};

struct State
{
    STATE state;
    json toJSON() const
    {
        return json{
            {"state", (int)(state)}};
    }
};

struct Animation
{
    float runtime_ms = 0;
    float timer_ms = 0;
    int pose = 0;
    int transition_ms;
    int pose_count;
    const TEXTURE_ASSET_ID *textures;
    bool loop = true;
    bool lock = false;
    bool destroy = false;

    json toJSON() const
    {
        return json{
            {"runtime_ms", runtime_ms},
            {"timer_ms", timer_ms},
            {"pose", pose},
            {"transition_ms", transition_ms},
            {"pose_count", pose_count},
            {"loop", loop},
            {"lock", lock},
            {"destroy", destroy}};
    }
};

struct PlantAnimation
{
    PLANT_ID id;

    json toJSON() const
    {
        return json{
            {"id", id}};
    }
};

// Tower
struct Tower
{
    float health; // health of the tower
    float damage; // damage of the tower
    float range;  // for vision / detection
    int timer_ms; // how often the tower attacks
    bool state;   // false (IDLE), true (ATTACK)
    PLANT_TYPE type;

    json toJSON() const
    {
        return json{
            {"health", health},
            {"damage", damage},
            {"range", range},
            {"timer_ms", timer_ms},
            {"state", state},
            {"type", type}
        };
    }
};

// Stucture to store collision information
struct Collision
{
    // Note, the first object is stored in the ECS container.entities
    Entity other; // the second object involved in the collision
    Collision(Entity &other) { this->other = other; };
    json toJSON() const
    {
        return json{
            {"other", other.id()}};
    }
};

struct MapTile
{
    json toJSON() const
    {
        return json{};
    };
};

struct TutorialTile
{
    json toJSON() const
    {
        return json{};
    };
};

struct TutorialSign
{
    json toJSON() const
    {
        return json{};
    };
};

struct ScorchedEarth
{
    json toJSON() const
    {
        return json{};
    };
};

struct Toolbar
{
    json toJSON() const
    {
        return json{};
    };
};

struct MoveWithCamera
{
    json toJSON() const
    {
        return json{};
    };
};

// Sets the brightness of the screen
// Includes HP and EXP parameters
struct ScreenState
{
    float darken_screen_factor = 0;

    float game_over_darken = -1;
    float game_over_counter_ms = 0;

    float hp_percentage = 1.0;
    float exp_percentage = 0.0;

    bool game_over = false;
    float lerp_timer = 0.0;

    // Screen shake parameters
    float shake_duration_ms = 0.f;
    float shake_intensity = 0.f;
    vec2 shake_offset = {0.f, 0.f};

    int cutscene = 0;
    int cg_index = 0;
    bool seed_cg = true;

    json toJSON() const
    {
        return json{
            {"darken_screen_factor", darken_screen_factor},
            {"game_over_darken", game_over_darken},
            {"game_over_counter_ms", game_over_counter_ms},
            {"hp_percentage", hp_percentage},
            {"exp_percentage", exp_percentage},
            {"game_over", game_over},
            {"lerp_timer", lerp_timer},
            {"shake_duration_ms", shake_duration_ms},
            {"shake_intensity", shake_intensity},
            {"shake_offset", {shake_offset.x, shake_offset.y}},
            {"cg_index", cg_index},
            {"cutscene", cutscene},
            {"seed_cg", seed_cg}};
    }
};

// used to hold grid line start and end positions
struct GridLine
{
    vec2 start_pos = {0, 0};
    vec2 end_pos = {10, 10}; // default to diagonal line
    json toJSON() const
    {
        return json{
            {"start_pos", {start_pos.x, start_pos.y}},
            {"end_pos", {end_pos.x, end_pos.y}}};
    }
};

// Single Vertex Buffer element for non-textured meshes (chicken.vs.glsl)
struct ColoredVertex
{
    vec3 position;
    vec3 color;
    json toJSON() const
    {
        return json{
            {"position", {position.x, position.y, position.z}},
            {"color", {color.x, color.y, color.z}}};
    }
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
    vec3 position;
    vec2 texcoord;
    json toJSON() const
    {
        return json{
            {"position", {position.x, position.y, position.z}},
            {"texcoord", {texcoord.x, texcoord.y}}};
    }
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
    static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
    vec2 original_size = {1, 1};
    std::vector<ColoredVertex> vertices;
    std::vector<uint16_t> vertex_indices;
    json toJSON() const
    {
        json vertices_json = json::array();
        for (const auto &vertex : vertices)
        {
            vertices_json.push_back(vertex.toJSON());
        }
        return json{
            {"original_size", {original_size.x, original_size.y}},
            {"vertices", vertices_json},
            {"vertex_indices", vertex_indices}};
    }
};

// Animation related components
struct DeathAnimation
{
    vec2 slide_direction;       // Direction to slide
    float alpha = 1.0f;         // Transparency (1.0 = solid, 0.0 = invisible)
    float duration_ms = 500.0f; // How long the animation lasts (Animation lasts 0.5 seconds)
    json toJSON() const
    {
        return json{
            {"slide_direction", {slide_direction.x, slide_direction.y}},
            {"alpha", alpha},
            {"duration_ms", duration_ms}};
    }
};

struct HitEffect
{
    float duration_ms = 200.0f; // How long the hit effect lasts
    bool is_white = true;       // For white flash effect
    json toJSON() const
    {
        return json{
            {"duration_ms", duration_ms},
            {"is_white", is_white}};
    }
};

struct Camera
{
    vec2 position = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2};
    float camera_width = CAMERA_VIEW_WIDTH;
    float camera_height = CAMERA_VIEW_HEIGHT;
    float lerp_factor = 0.1f;
    json toJSON() const
    {
        return json{
            {"position", {position.x, position.y}},
            {"camera_width", camera_width},
            {"camera_height", camera_height},
            {"lerp_factor", lerp_factor}};
    }
};

struct CustomButton
{
    BUTTON_ID type;
    vec2 position;
    json toJSON() const
    {
        return json{}; // don't use it, just for compile purpose
    }
};
// Update your existing Particle struct

struct Particle
{
    glm::vec2 Position;
    glm::vec2 Velocity;
    glm::vec4 Color;
    float Life;
    float MaxLife;

    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f), MaxLife(0.0f) {}

    json toJSON() const
    {
        return json{
            {"Position", {Position.x, Position.y}},
            {"Velocity", {Velocity.x, Velocity.y}},
            {"Color", {Color.x, Color.y, Color.z, Color.a}},
            {"Life", Life},
            {"MaxLife", MaxLife}
        };
    }
};

struct ParticleGenerator
{
    std::string type;              // Type of effect (blood, fire, etc.)
    unsigned int amount;           // Maximum number of particles
    float spawnInterval;           // Time between spawning particles
    float timer;                   // Current timer
    std::vector<Entity> particles; // List of particle entities
    bool isActive;                 // Whether generator is active
    float duration_ms;             // How long this generator remains active (-1 for infinite)
    Entity follow_entity = Entity(); // Entity to follow (if any) - Fixed NULL to Entity()
    float max_visible_distance = 800.0f; // Maximum distance from player to be visible
    
    ParticleGenerator()
        : type("default"), amount(100), spawnInterval(0.1f), timer(0.0f),
          particles(), isActive(true), duration_ms(-1.0f) {}
    
    json toJSON() const
    {
        return json{};
    }
};

// Text
struct Text {
	std::string text;
	vec2 pos;
	float size;
	vec3 color = vec3(0.0f, 0.0f, 0.0f);

    // compile purpose, not gonna save it
    json toJSON() const
    {
        return json{};
    }
};

struct CG
{
    // compile purpose, not gonna save it
    json toJSON() const
    {
        return json{};
    }
};

struct Squad
{
    int squad_id;
    std::vector<Entity> archers;
    std::vector<Entity> orcs;
    std::vector<Entity> knights;
    vec2 formation_center;
    vec2 last_player_pos = {0, 0}; // Used to track player movement
    float coordination_timer = 0.f;
    bool is_active = true;

    enum class Formation
    {
        LINE,
        DEFENSIVE,
        FLANKING
    };

    Formation current_formation = Formation::DEFENSIVE;

    json toJSON() const
    {
        json archersJson = json::array();
        json orcsJson = json::array();
        json knightsJson = json::array();
        for (const auto& archer : archers)
            archersJson.push_back(archer.id());
        for (const auto& orc : orcs)
            orcsJson.push_back(orc.id());
        for (const auto& knight : knights)
            knightsJson.push_back(knight.id());
        return json{
            {"squad_id", squad_id},
            {"archers", archersJson},
            {"orcs", orcsJson},
            {"knights", knightsJson},
            {"formation_center", {formation_center.x, formation_center.y}},
            {"last_player_pos", {last_player_pos.x, last_player_pos.y}},
            {"coordination_timer", coordination_timer},
            {"is_active", is_active}
        };
    }
};

struct Slow
{
    float value;
    int timer_ms;
    json toJSON() const
    {
        return json{
            {"value", value},
            {"timer_ms", timer_ms}
        };
    }
};


struct ElectricityData
{
    float noise_seed;
    vec2 curve_ctrl1;
    vec2 curve_ctrl2;
    
    // Additional fields can be added as needed
    
    json toJSON() const
    {
        return json{
            {"noise_seed", noise_seed},
            {"curve_ctrl1", {curve_ctrl1.x, curve_ctrl1.y}},
            {"curve_ctrl2", {curve_ctrl2.x, curve_ctrl2.y}}
        };
    }
};