#include "particle_system.hpp"
#include "render_system.hpp"
#include <algorithm>

ParticleSystem::ParticleSystem() : gen(rd()), dist(0.0f, 1.0f)
{
}

void ParticleSystem::init(RenderSystem *renderer_arg)
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
        ParticleGenerator &generator = registry.particleGenerators.get(entity);

        if (!generator.isActive)
            continue;
        // Follow player if needed (for level_up effect)
        if (generator.follow_entity != NULL &&
            registry.motions.has(generator.follow_entity) &&
            registry.motions.has(entity))
        {
            // Update generator position to follow the entity
            Motion &follow_motion = registry.motions.get(generator.follow_entity);
            registry.motions.get(entity).position = follow_motion.position;
        }
        // Check if the generator has expired
        if (generator.duration_ms > 0)
        {
            generator.duration_ms -= elapsed_ms;
            if (generator.duration_ms <= 0)
            {
                generator.isActive = false;
                continue;
            }
        }

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
                [](Entity e)
                {
                    return !registry.particles.has(e) ||
                           registry.particles.get(e).Life <= 0.0f;
                }),
            generator.particles.end());
    }
}

void ParticleSystem::updateParticles(float elapsed_ms)
{
    float delta_s = elapsed_ms / 1000.0f;

    for (Entity entity : registry.particles.entities)
    {
        Particle &particle = registry.particles.get(entity);

        // Update life
        particle.Life -= delta_s;

        // Remove dead particles
        if (particle.Life <= 0.0f)
        {
            registry.remove_all_components_of(entity);
            continue;
        }


        
        // Update position based on velocity
        particle.Position += particle.Velocity * delta_s;

        // Update color based on life ratio
        float life_ratio = particle.Life / particle.MaxLife;
        particle.Color.a = life_ratio; // Fade out

        // Update motion component for rendering
        if (registry.motions.has(entity))
        {
            Motion &motion = registry.motions.get(entity);
            motion.position = particle.Position;
            motion.scale = vec2(10.0f * life_ratio);
        }
    }
}

Entity ParticleSystem::createParticle(const ParticleGenerator &generator, vec2 position)
{
    Entity entity = Entity();

    // Create particle component
    Particle &particle = registry.particles.emplace(entity);
    particle.Position = position;

    // Randomize velocity based on type
    if (generator.type == "blood_sprite")
    {
        // Get sprite dimensions from the generator entity
        vec2 sprite_size = {50.0f, 50.0f}; // Default size in case motion isn't available

        for (Entity gen_entity : registry.particleGenerators.entities)
        {
            if (registry.particleGenerators.has(gen_entity) &&
                &registry.particleGenerators.get(gen_entity) == &generator &&
                registry.motions.has(gen_entity))
            {
                sprite_size = registry.motions.get(gen_entity).scale;
                break;
            }
        }

        // Generate position across the whole sprite area
        // Offset position to be within the sprite bounds
        float x_offset = randomFloat(-sprite_size.x / 2, sprite_size.x / 2);
        float y_offset = randomFloat(-sprite_size.y / 2, sprite_size.y / 2);

        particle.Position.x += x_offset;
        particle.Position.y += y_offset;

        // Blood particles fall downward with slight horizontal variation
        float angle = randomFloat(M_PI / 2 - 0.3f, M_PI / 2 + 0.3f); // Mostly downward
        float speed = randomFloat(30.0f, 80.0f);
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed};

        // Deep red color with varying opacity
        particle.Color = {
            randomFloat(0.6f, 0.8f), // Dark red
            randomFloat(0.0f, 0.1f), // Almost no green
            randomFloat(0.0f, 0.1f), // Almost no blue
            randomFloat(0.7f, 1.0f)  // Varying opacity
        };

        // Varying lifetimes for more natural effect
        particle.MaxLife = randomFloat(0.6f, 1.2f);
    }
    else if (generator.type == "fire")
    {
        float angle = randomFloat(-M_PI / 4, M_PI / 4) - M_PI / 2; // Mostly upward
        float speed = randomFloat(30.0f, 60.0f);
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed};
        particle.Color = {1.0f, randomFloat(0.3f, 0.7f), 0.0f, 1.0f}; // Orange-red
        particle.MaxLife = randomFloat(0.6f, 1.0f);
    }
    else if (generator.type == "seed_growth")
    {
        // Get sprite dimensions from the generator entity
        vec2 sprite_size = {50.0f, 50.0f}; // Default size in case motion isn't available
        for (Entity gen_entity : registry.particleGenerators.entities)
        {
            if (registry.particleGenerators.has(gen_entity) &&
                &registry.particleGenerators.get(gen_entity) == &generator &&
                registry.motions.has(gen_entity))
            {
                sprite_size = registry.motions.get(gen_entity).scale;
                break;
            }
        }

        // Better distribution pattern for full coverage
        // Use grid-based positioning with jitter to avoid patterns
        int cells = 5; // 5x5 grid for distribution
        int cell_x = (int)randomFloat(0, cells);
        int cell_y = (int)randomFloat(0, cells);

        // Calculate position within the grid cell (with jitter)
        float x_percent = (cell_x + randomFloat(0.2f, 0.8f)) / cells;
        float y_percent = (cell_y + randomFloat(0.2f, 0.8f)) / cells;

        // Map to sprite coordinates
        float x_offset = (x_percent * 2.0f - 1.0f) * sprite_size.x * 0.5f;
        float y_offset = (y_percent * 2.0f - 1.0f) * sprite_size.y * 0.5f;

        particle.Position.x += x_offset;
        particle.Position.y += y_offset;

        // Slower upward movement to keep particles visible around the seed longer
        float angle = randomFloat(-M_PI / 6, M_PI / 6) - M_PI / 2; // Mostly upward
        float speed = randomFloat(15.0f, 30.0f);                   // Slower speed
        particle.Velocity = {cos(angle) * speed, sin(angle) * speed};

        // Add slight horizontal drift for natural movement
        particle.Velocity.x += randomFloat(-5.0f, 5.0f);

        // Green color with varying shades
        particle.Color = {
            randomFloat(0.0f, 0.2f), // Low red
            randomFloat(0.7f, 1.0f), // High green
            randomFloat(0.0f, 0.4f), // Low blue
            randomFloat(0.7f, 1.0f)  // Varying opacity
        };

        particle.MaxLife = randomFloat(0.6f, 1.5f); // Longer lifetime for better coverage
    }
    else if (generator.type == "level_up")
    {
        // Get sprite dimensions from the generator entity
        vec2 sprite_size = {50.0f, 50.0f}; // Default size
        for (Entity gen_entity : registry.particleGenerators.entities)
        {
            if (registry.particleGenerators.has(gen_entity) &&
                &registry.particleGenerators.get(gen_entity) == &generator &&
                registry.motions.has(gen_entity))
            {
                sprite_size = registry.motions.get(gen_entity).scale;
                break;
            }
        }

        // Calculate distance from center and angle for aura effect
        float radius = (sprite_size.x > sprite_size.y ? sprite_size.x : sprite_size.y) * 0.7f;
        float angle = randomFloat(0.0f, 2.0f * M_PI);

        // Position particles in a circle around the player
        particle.Position.x = position.x + cos(angle) * radius;
        particle.Position.y = position.y + sin(angle) * radius;

        // Slow circular movement for aura effect
        float orbit_speed = randomFloat(10.0f, 30.0f);
        float orbit_direction = (randomFloat(0.0f, 1.0f) > 0.5f) ? 1.0f : -1.0f; // Clockwise or counter

        // Tangential velocity for circular motion
        particle.Velocity = {
            -sin(angle) * orbit_speed * orbit_direction,
            cos(angle) * orbit_speed * orbit_direction};

        // Add small inward/outward drift
        float radial_drift = randomFloat(-5.0f, 15.0f);
        particle.Velocity.x += cos(angle) * radial_drift;
        particle.Velocity.y += sin(angle) * radial_drift;

        // Golden/yellow color with pulsing effect
        particle.Color = {
            1.0f,                    // Full red
            randomFloat(0.8f, 1.0f), // High green
            randomFloat(0.0f, 0.3f), // Low blue
            randomFloat(0.7f, 1.0f)  // Varying opacity
        };

        particle.MaxLife = randomFloat(0.5f, 1.2f);
    }
    // In createParticle function, add this case:
    else if (generator.type == "electricity_line")
    {
        // Get start and end points
        vec2 start_pos = position;
        vec2 end_pos = start_pos;

        // Find generator entity
        Entity generator_entity = Entity();
        for (Entity e : registry.particleGenerators.entities)
        {
            if (&registry.particleGenerators.get(e) == &generator)
            {
                generator_entity = e;
                break;
            }
        }

        // Find the end position
        if (registry.motions.has(generator_entity))
        {
            for (int i = 0; i < registry.motions.size(); i++)
            {
                if (registry.motions.entities[i] == generator_entity && i > 0)
                {
                    end_pos = registry.motions.components[i].position;
                    break;
                }
            }
        }

        // Get the main path vector
        vec2 path = end_pos - start_pos;
        float path_length = length(path);
        vec2 path_dir = normalize(path);
        vec2 perpendicular = vec2(-path_dir.y, path_dir.x);

        // Create particles along the entire path (assign them segments)
        int segment_id = rand() % 5;       // Create multiple bolt segments
        float t = randomFloat(0.0f, 1.0f); // Position along path

        // Generate a bezier control point for curve
        vec2 control_point = start_pos + path * 0.5f +
                             perpendicular * randomFloat(-50.0f, 50.0f);

        // Calculate position using quadratic bezier
        float u = 1.0f - t;
        particle.Position = u * u * start_pos + 2 * u * t * control_point + t * t * end_pos;

        // Apply perlin noise to the position for more natural jaggedness
        // Using glm's simplex noise if available, otherwise use our own noise
        float noise_scale = 0.1f;
        float noise_strength = 20.0f;
        float noise_offset = randomFloat(0.0f, 100.0f); // Different for each bolt

        // Apply noise perpendicular to the path
        float noise_value = sin(t * 10.0f + noise_offset) * noise_strength;
        particle.Position += perpendicular * noise_value;

        // Velocity along the path
        particle.Velocity = path_dir * randomFloat(200.0f, 400.0f);

        // Electric blue color with variation
        float brightness = randomFloat(0.8f, 1.0f);
        float blue_tint = randomFloat(0.9f, 1.0f);
        particle.Color = {
            randomFloat(0.6f, 0.9f) * brightness, // Blue-white
            randomFloat(0.7f, 1.0f) * brightness, // with slight variation
            1.0f * brightness * blue_tint,        // Full blue
            randomFloat(0.7f, 1.0f)               // Varying opacity
        };

        // Store the segment ID and t-value for connected rendering
        particle.Life = randomFloat(0.05f, 0.15f);
        particle.MaxLife = particle.Life;
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
    Motion &motion = registry.motions.emplace(entity);
    motion.position = particle.Position;
    motion.scale = vec2(10.0f); // Default size

    // Add render request using your existing textures and effects
    // Note: You'll need to add PARTICLE to your TEXTURE_ASSET_ID and EFFECT_ASSET_ID enums
    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PARTICLE, EFFECT_ASSET_ID::PARTICLE, GEOMETRY_BUFFER_ID::SPRITE});

    // Add color for rendering (if your renderer uses this)
    registry.colors.emplace(entity) = vec3(particle.Color);

    return entity;
}

Entity ParticleSystem::createBloodEffect(vec2 position, vec2 sprite_size)
{
    Entity entity = Entity();

    // Add motion component
    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;

    // Add generator component
    ParticleGenerator &generator = registry.particleGenerators.emplace(entity);
    generator.type = "blood_sprite"; // New type for whole-sprite blood
    generator.amount = 60;           // More particles needed
    generator.spawnInterval = 0.01f; // Burst quickly
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = 300.0f; // Slightly longer duration

    // Store the sprite size in the scale field for later use
    motion.scale = sprite_size;

    return entity;
}

Entity ParticleSystem::createFireEffect(vec2 position)
{
    Entity entity = Entity();

    // Add motion component
    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;

    // Add generator component
    ParticleGenerator &generator = registry.particleGenerators.emplace(entity);
    generator.type = "fire";
    generator.amount = 100;
    generator.spawnInterval = 0.05f; // Continuous fire
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = -1.0f; // Infinite duration until explicitly stopped

    return entity;
}

Entity ParticleSystem::createSeedGrowthEffect(vec2 position, vec2 sprite_size)
{
    Entity entity = Entity();

    // Add motion component
    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = sprite_size; // Store size for proper coverage

    // Add generator component
    ParticleGenerator &generator = registry.particleGenerators.emplace(entity);
    generator.type = "seed_growth";
    generator.amount = 80;           // More particles for better coverage
    generator.spawnInterval = 0.03f; // Faster emission rate
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = 5000.0f; // Same as seed growth time (5 seconds)

    return entity;
}

Entity ParticleSystem::createLevelUpEffect(vec2 position, vec2 sprite_size)
{
    Entity entity = Entity();

    // Add motion component
    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = sprite_size; // Store size for aura calculation

    // Add generator component
    ParticleGenerator &generator = registry.particleGenerators.emplace(entity);
    generator.type = "level_up";
    generator.amount = 120;
    generator.spawnInterval = 0.005f;
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = 1500.0f;

    // Store the player entity for following
    generator.follow_entity = registry.players.entities[0];

    return entity;
}

Entity ParticleSystem::createElectricityEffect(vec2 start_point, vec2 end_point, float width, float duration_ms)
{
    Entity entity = Entity();

    // Add motion component (parent controller)
    Motion &motion = registry.motions.emplace(entity);
    motion.position = start_point;
    motion.scale = vec2(length(end_point - start_point), width);
    motion.angle = atan2(end_point.y - start_point.y, end_point.x - start_point.x) * 180.0f / M_PI;

    // Add particle generator component
    ParticleGenerator &generator = registry.particleGenerators.emplace(entity);
    generator.type = "electricity_line"; // New type for continuous lightning
    generator.amount = 50;               // Fewer particles, but they'll form a continuous line
    generator.spawnInterval = 0.02f;     // Faster spawn rate for smoother animation
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = duration_ms;

    // Store end point for path calculation
    generator.follow_entity = Entity(); // Using default constructor

    // Store end_point in duplicate motion
    Motion temp;
    temp.position = end_point;
    registry.motions.emplace_with_duplicates(entity, temp);

    return entity;
}
float ParticleSystem::randomFloat(float min, float max)
{
    return min + dist(gen) * (max - min);
}

void ParticleSystem::stopEffect(Entity generator_entity)
{
    if (registry.particleGenerators.has(generator_entity))
    {
        ParticleGenerator &generator = registry.particleGenerators.get(generator_entity);
        generator.isActive = false;
    }
}