#include "spawn_manager.hpp"
#include "world_init.hpp"
#include <iostream>

SpawnManager::SpawnManager() : wave_timer_ms(0.f),
                               next_wave_ms(WAVE_INTERVAL_MS),
                               zombies_per_wave(INITIAL_ZOMBIES_PER_WAVE),
                               current_wave(0),
                               is_game_started(false)
{
    // Initialize RNG
    rng = std::default_random_engine(std::random_device()());
    uniform_dist = std::uniform_real_distribution<float>(0.f, 1.f);

    initialize_spawn_points();
}

void SpawnManager::initialize_spawn_points()
{
    const float OFFSET = -150.f; // Offset from edges for visibility

    // Create 8 spawn points, 2 on each edge
    // Top edge
    for (int i = 1; i <= 4; i++) {
        float x_pos = (MAP_WIDTH_PX * i) / 5.0f; 
        spawn_points.push_back({{x_pos, OFFSET}, true});
    }

    // Right edge
    for (int i = 1; i <= 4; i++) {
        float y_pos = (MAP_HEIGHT_PX * i) / 5.0f; 
        spawn_points.push_back({{MAP_WIDTH_PX - OFFSET, y_pos}, true});
    }

    // Bottom edge
    for (int i = 1; i <= 4; i++) {
        float x_pos = (MAP_WIDTH_PX * i) / 5.0f;
        spawn_points.push_back({{x_pos, MAP_HEIGHT_PX - OFFSET}, true});
    }

    // Left edge
    for (int i = 1; i <= 4; i++) {
        float y_pos = (MAP_HEIGHT_PX * i) / 5.0f;
        spawn_points.push_back({{OFFSET, y_pos}, true});
    }
}

void SpawnManager::step(float elapsed_ms, RenderSystem *renderer)
{
    // If game isn't started or in test mode, don't do anything
    if (!is_game_started || test_mode) {
        return;
    }
    
    // Update the timer for spawning
    wave_timer_ms += elapsed_ms;
    
    // Time to spawn an enemy?
    if (wave_timer_ms >= next_wave_ms)
    {
        // Spawn a single enemy
        spawn_enemy(renderer);
        
        // Reset timer
        wave_timer_ms = 0.f;
    }
}

void SpawnManager::generate_wave(RenderSystem *renderer)
{
    current_wave++;
    std::cout << "=== Wave " << current_wave << " Starting ===" << std::endl;

    // Spawn zombies at random points until we reach the target number
    for (int i = 0; i < zombies_per_wave; i++)
    {
        // Get a random spawn point index
        int random_point = (int)(uniform_dist(rng) * spawn_points.size());
        vec2 spawn_pos = spawn_points[random_point].position;
        if (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) < 0.3f)
        {
            createZombie(renderer, spawn_pos);
        }
        else
        {
            createSkeleton(renderer, spawn_pos);
        }
    }

    if (IS_WAVE_MODE_LINEAR)
    {
        zombies_per_wave = (int)(zombies_per_wave + WAVE_SCALING_FACTOR);
    }
    else {
        zombies_per_wave = (int)(zombies_per_wave * WAVE_SCALING_FACTOR);
    }
    std::cout << "Next wave will have " << zombies_per_wave << " zombies" << std::endl;
}

void SpawnManager::reset()
{
    std::cout << "SpawnManager: Reset called!" << std::endl;
    wave_timer_ms = 0.f;
    next_wave_ms = WAVE_INTERVAL_MS;
    zombies_per_wave = INITIAL_ZOMBIES_PER_WAVE;
    current_wave = 0;
    is_game_started = false;
}

void SpawnManager::start_game()
{
    std::cout << "SpawnManager: Game started!" << std::endl;
    is_game_started = true;
    wave_timer_ms = 0.f;
}

void SpawnManager::set_test_mode(bool enabled)
{
    test_mode = enabled;
    std::cout << "Test mode " << (enabled ? "enabled" : "disabled") << std::endl;
}

void SpawnManager::spawn_enemy(RenderSystem* renderer)
{
    // Get a random spawn point
    int random_point = (int)(uniform_dist(rng) * spawn_points.size());
    vec2 spawn_pos = spawn_points[random_point].position;
    
    // Fixed 30% chance for skeleton
    const float SKELETON_CHANCE = 0.3f;
    
    if (uniform_dist(rng) < SKELETON_CHANCE) {
        createSkeleton(renderer, spawn_pos);
    } else {
        createZombie(renderer, spawn_pos);
    }
}