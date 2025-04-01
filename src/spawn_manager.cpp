#include "spawn_manager.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
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
    if (!is_game_started || test_mode || WorldSystem::get_game_screen() == GAME_SCREEN_ID::TUTORIAL) {
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
            createOrc(renderer, spawn_pos);
        }
        else
        {
            createSkeletonArcher(renderer, spawn_pos);
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

    // Get the current day from WorldSystem
    int current_day = WorldSystem::get_current_day();
    
    // Spawn enemy based on the current day progression
    spawnEnemyByDay(renderer, spawn_pos, current_day);
}

Entity SpawnManager::spawnEnemyByDay(RenderSystem* renderer, vec2 spawn_pos, int current_day)
{
    // Progressive enemy introduction based on day number
    std::vector<std::function<Entity()>> available_enemies;
    float elite_chance = 0.0f;
    
    // Always have Orc as the basic enemy from day 1
    available_enemies.push_back([&]() { return createOrc(renderer, spawn_pos); });
    
    // Day 2: Add Skeleton
    if (current_day >= 2) {
        available_enemies.push_back([&]() { return createSkeleton(renderer, spawn_pos); });
    }
    
    // Day 3: Add OrcElite
    if (current_day >= 3) {
        available_enemies.push_back([&]() { return createOrcElite(renderer, spawn_pos); });
        elite_chance = 0.1f; // 10% chance for elite versions
    }
    
    // Day 4: Add Werewolf
    if (current_day >= 4) {
        available_enemies.push_back([&]() { return createWerewolf(renderer, spawn_pos); });
    }
    
    // Day 5: Add SkeletonArcher
    if (current_day >= 5) {
        available_enemies.push_back([&]() { return createSkeletonArcher(renderer, spawn_pos); });
        elite_chance = 0.15f; // 15% chance for elite versions
    }
    
    // Day 6: Add Werebear
    if (current_day >= 6) {
        available_enemies.push_back([&]() { return createWerebear(renderer, spawn_pos); });
    }
    
    // Day 7: Add Slime
    if (current_day >= 7) {
        available_enemies.push_back([&]() { return createSlime(renderer, spawn_pos); });
        elite_chance = 0.2f; // 20% chance for elite versions
    }
    
    // Day 8: Add OrcRider
    if (current_day >= 8) {
        available_enemies.push_back([&]() { return createOrcRider(renderer, spawn_pos); });
        elite_chance = 0.25f; // 25% chance for elite versions
    }

    // Once all enemies are introduced, use a weighted random distribution
    if (current_day >= 9) {
        // After day 9, use the original random distribution but with adjusted rates
        float prob = uniform_dist(rng);
        
        if (prob < 0.25f) {
            return createSkeletonArcher(renderer, spawn_pos);
        }
        else if (prob < 0.35f) {
            return createOrcElite(renderer, spawn_pos);
        }
        else if (prob < 0.45f) {
            return createSkeleton(renderer, spawn_pos);
        }
        else if (prob < 0.55f) {
            return createWerewolf(renderer, spawn_pos);
        }
        else if (prob < 0.65f) {
            return createWerebear(renderer, spawn_pos);
        }
        else if (prob < 0.75f) {
            return createSlime(renderer, spawn_pos);
        }
        else {
            return createOrc(renderer, spawn_pos);
        }
    }
    
    // For days 1-7, select from available enemies list
    int enemy_idx = (int)(uniform_dist(rng) * available_enemies.size());
    return available_enemies[enemy_idx]();
}