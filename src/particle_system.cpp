#include "particle_system.hpp"
#include "render_system.hpp"
#include <algorithm>

ParticleSystem::ParticleSystem() : gen(rd()), dist(0.0f, 1.0f)
{
}

void ParticleSystem::init(RenderSystem* renderer_arg)
{
    this->renderer = renderer_arg;
}

void ParticleSystem::step(float elapsed_ms)
{
    // Update generators first
    updateParticleGenerators(elapsed_ms);
    
    // Then update all particles
    updateParticles(elapsed_ms);
}

void ParticleSystem::updateParticleGenerators(float elapsed_ms)
{
    for (Entity entity : registry.particleGenerators.entities)
    {
        ParticleGenerator& generator = registry.particleGenerators.get(entity);
        
        if (!generator.isActive)
            continue;
            
        // Check if we need to spawn more particles
        generator.timer += elapsed_ms;
        
        if (generator.timer >= generator.spawnInterval * 1000.0f) // Convert to ms
        {
            generator.timer = 0.0f;
            
            // Check if we have room for more particles
            if (generator.particles.size() < generator.amount)
            {
                // Get position from the entity's motion
                vec2 position = {0.0f, 0.0f};
                if (registry.motions.has(entity))
                {
                    position = registry.motions.get(entity).position;
                }
                
                // Create a new particle
                Entity particle = createParticle(generator, position);
                
                // Add to generator's list
                generator.particles.push_back(particle);
            }
        }
        
        // Remove dead particles from the list
        generator.particles.erase(
            std::remove_if(
                generator.particles.begin(),
                generator.particles.end(),
                [](Entity e) { 
                    return !registry.particles.has(e) || 
                           registry.particles.get(e).Life <= 0.0f; 
                }
            ),
            generator.particles.end()
        );
    }
}

void ParticleSystem::updateParticles(float elapsed_ms)
{
    float delta_s = elapsed_ms / 1000.0f;
    
    for (Entity entity : registry.particles.entities)
    {
        Particle& particle = registry.particles.get(entity);
        
        // Update life
        particle.Life -= delta_s;
        
        // Remove dead particles
        if (particle.Life <= 0.0f)
        {
            registry.remove_all_components_of(entity);
            continue;
        }
        
        // Update position
        particle.Position += particle.Velocity * delta_s;
        
        // Update color based on life ratio
        float life_ratio = particle.Life / particle.MaxLife;
        particle.Color.a = life_ratio; // Fade out
        
        // Update motion component for rendering
        if (registry.motions.has(entity))
        {
            Motion& motion = registry.motions.get(entity);
            motion.position = particle.Position;
            motion.scale = vec2(10.0f * life_ratio); // Shrink as it ages
        }
    }
}

Entity ParticleSystem::createParticle(const ParticleGenerator& generator, vec2 position)
{
    Entity entity = Entity();
    
    // Create particle component
    Particle& particle = registry.particles.emplace(entity);
    particle.Position = position;
    
    // Randomize velocity based on type
    if (generator.type == "blood")
    {
        float angle = randomFloat(0.0f, 2.0f * M_PI);
        float speed = randomFloat(20.0f, 80.0f);
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed + 50.0f}; // Bias downward
        particle.Color = {0.7f, 0.0f, 0.0f, 1.0f}; // Dark red
        particle.MaxLife = randomFloat(0.5f, 0.8f);
    }
    else if (generator.type == "fire")
    {
        float angle = randomFloat(-M_PI/4, M_PI/4) - M_PI/2; // Mostly upward
        float speed = randomFloat(30.0f, 60.0f);
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed};
        particle.Color = {1.0f, randomFloat(0.3f, 0.7f), 0.0f, 1.0f}; // Orange-red
        particle.MaxLife = randomFloat(0.6f, 1.0f);
    }
    else
    {
        // Default
        float angle = randomFloat(0.0f, 2.0f * M_PI);
        float speed = randomFloat(10.0f, 30.0f);
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed};
        particle.Color = {1.0f, 1.0f, 1.0f, 1.0f};
        particle.MaxLife = randomFloat(0.5f, 1.0f);
    }
    
    particle.Life = particle.MaxLife;
    
    // Add motion for rendering
    Motion& motion = registry.motions.emplace(entity);
    motion.position = particle.Position;
    motion.scale = vec2(10.0f); // Default size
    
    // Add render request using your existing textures and effects
    // Note: You'll need to add PARTICLE to your TEXTURE_ASSET_ID and EFFECT_ASSET_ID enums
    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PARTICLE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    );
    
    // Add color for rendering (if your renderer uses this)
    registry.colors.emplace(entity) = vec3(particle.Color);
    
    return entity;
}

Entity ParticleSystem::createBloodEffect(vec2 position)
{
    Entity entity = Entity();
    
    // Add motion component
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    
    // Add generator component
    ParticleGenerator& generator = registry.particleGenerators.emplace(entity);
    generator.type = "blood";
    generator.amount = 30;
    generator.spawnInterval = 0.01f; // Burst quickly
    generator.timer = 0.0f;
    generator.isActive = true;
    
    return entity;
}

Entity ParticleSystem::createFireEffect(vec2 position)
{
    Entity entity = Entity();
    
    // Add motion component
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    
    // Add generator component
    ParticleGenerator& generator = registry.particleGenerators.emplace(entity);
    generator.type = "fire";
    generator.amount = 100;
    generator.spawnInterval = 0.05f; // Continuous fire
    generator.timer = 0.0f;
    generator.isActive = true;
    
    return entity;
}

float ParticleSystem::randomFloat(float min, float max)
{
    return min + dist(gen) * (max - min);
}