#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include <vector>
#include <random>

class SpawnManager {
public:
    SpawnManager();
    void step(float elapsed_ms, RenderSystem* renderer);  // Pass renderer in step
    void generate_wave(RenderSystem* renderer);  // Keep this for manual wave generation
    void reset();   // Reset the spawn manager
    void start_game(); // Start the game for spawning zombies

    void set_test_mode(bool enabled);
    bool is_test_mode() const { return test_mode; }
    int current_wave;

private:
    struct SpawnPoint {
        vec2 position;
        bool is_active;
    };

    void initialize_spawn_points();
     
    std::vector<SpawnPoint> spawn_points;
    float wave_timer_ms;
    float next_wave_ms;
    int zombies_per_wave;
    
    
    // RNG
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;
    
    // Constants
    static constexpr float WAVE_INTERVAL_MS = 5000.0f;
    static constexpr int INITIAL_ZOMBIES_PER_WAVE = 2;
    static constexpr float WAVE_SCALING_FACTOR = 1.5f;

    // Keep track of whether the game has really started
    bool is_game_started = false;

    // Test mode for manual wave generation
    bool test_mode = false;
};