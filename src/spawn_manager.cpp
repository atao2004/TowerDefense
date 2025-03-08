#include "spawn_manager.hpp"
#include "world_init.hpp"
#include <iostream>

SpawnManager::SpawnManager() : wave_timer_ms(0.f),
                               next_wave_ms(WAVE_INTERVAL_MS),
                               zombies_per_wave(INITIAL_ZOMBIES_PER_WAVE),
                               current_wave(0)
{
    // Initialize RNG
    rng = std::default_random_engine(std::random_device()());
    uniform_dist = std::uniform_real_distribution<float>(0.f, 1.f);

    initialize_spawn_points();
}

void SpawnManager::initialize_spawn_points()
{
    const float OFFSET = 50.f; // Offset from edges for visibility

    // Create 8 spawn points, 2 on each edge
    // Top edge
    for (int i = 1; i <= 4; i++) {
        float x_pos = (WINDOW_WIDTH_PX * i) / 5.0f; 
        spawn_points.push_back({{x_pos, OFFSET}, true});
    }

    // Right edge
    for (int i = 1; i <= 4; i++) {
        float y_pos = (WINDOW_HEIGHT_PX * i) / 5.0f; 
        spawn_points.push_back({{WINDOW_WIDTH_PX - OFFSET, y_pos}, true});
    }

    // Bottom edge
    for (int i = 1; i <= 4; i++) {
        float x_pos = (WINDOW_WIDTH_PX * i) / 5.0f;
        spawn_points.push_back({{x_pos, WINDOW_HEIGHT_PX - OFFSET}, true});
    }

    // Left edge
    for (int i = 1; i <= 4; i++) {
        float y_pos = (WINDOW_HEIGHT_PX * i) / 5.0f;
        spawn_points.push_back({{OFFSET, y_pos}, true});
    }
}

void SpawnManager::step(float elapsed_ms, RenderSystem *renderer)
{

    if (!is_game_started)
    {
        std::cout << "Game not started yet, skipping wave generation" << std::endl;
        return;
    }

    if (!test_mode)
    {
        wave_timer_ms += elapsed_ms;
        if (wave_timer_ms >= next_wave_ms)
        {
            generate_wave(renderer);
            wave_timer_ms = 0.f;
            next_wave_ms = WAVE_INTERVAL_MS;
        }
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
        if (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) < 0.7f)
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