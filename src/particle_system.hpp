#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include <random>

class RenderSystem;

class ParticleSystem
{
public:
    ParticleSystem();

    // Initialize the system
    void init(RenderSystem *renderer);

    // Update all particles and generators
    void step(float elapsed_ms);

    // Create different particle effects
    static Entity createBloodEffect(vec2 position, vec2 sprite_size);
    static Entity createFireEffect(vec2 position);
    static Entity createSeedGrowthEffect(vec2 position, vec2 sprite_size); 
    static Entity createLevelUpEffect(vec2 position, vec2 sprite_size);
    static Entity createAOEEffect(vec2 position, vec2 sprite_size, int duration, Entity target, std::string type);

private:
    // Random number generator
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;

    // Keep reference to renderer
    RenderSystem *renderer;

    // Create a new particle
    Entity createParticle(const ParticleGenerator &generator, vec2 position);

    // Helper functions
    void updateParticleGenerators(float elapsed_ms);
    void updateParticles(float elapsed_ms);
    float randomFloat(float min, float max);

    void stopEffect(Entity generator_entity);
    
};