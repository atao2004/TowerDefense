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
        if (generator.follow_entity != Entity() &&
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

        // Determine particle type from its generator
        std::string particle_type = "default";
        Entity generator_entity;

        // Find the generator that created this particle
        for (Entity gen_entity : registry.particleGenerators.entities)
        {
            ParticleGenerator &generator = registry.particleGenerators.get(gen_entity);

            // Use a standard loop instead of std::find to avoid potential operator== issues
            bool found = false;
            for (unsigned int i = 0; i < generator.particles.size(); i++)
            {
                if (generator.particles[i].id() == entity.id())
                {
                    found = true;
                    break;
                }
            }

            if (found)
            {
                particle_type = generator.type;
                generator_entity = gen_entity;
                break;
            }
        }

        // Special handling for electricity particles
        if (particle_type == "electricity_line")
        {
            // Flickering effect
            if (randomFloat(0.0f, 1.0f) < 0.3f) // 30% chance per frame to flicker
            {
                // Randomly adjust brightness for flickering
                float brightness = randomFloat(0.8f, 1.2f);
                particle.Color.r *= brightness;
                particle.Color.g *= brightness;
                particle.Color.b *= brightness;

                // Occasionally create bright flash
                if (randomFloat(0.0f, 1.0f) < 0.1f)
                {
                    brightness = randomFloat(1.5f, 2.0f);
                    particle.Color = vec4(
                        brightness * 0.8f, // Bright blue-white
                        brightness * 0.9f,
                        brightness,
                        particle.Color.a);
                }
            }

            // Make electricity particles keep higher alpha
            particle.Color.a = 0.7f + 0.3f * (particle.Life / particle.MaxLife);

            // Add jitter to velocity for more chaotic movement
            particle.Velocity.x += randomFloat(-80.0f, 80.0f) * delta_s;
            particle.Velocity.y += randomFloat(-80.0f, 80.0f) * delta_s;

            // Update position with jittery velocity
            particle.Position += particle.Velocity * delta_s;

            // Update motion component for rendering - electricity particles are larger
            if (registry.motions.has(entity))
            {
                Motion &motion = registry.motions.get(entity);
                motion.position = particle.Position;

                // Vary size for electricity with pulsing effect
                float size_factor = 15.0f + sin(particle.Life * 30.0f) * 5.0f;
                motion.scale = vec2(size_factor);

                // Randomly rotate for more dynamic appearance
                motion.angle += randomFloat(-10.0f, 10.0f);
            }
        }
        else
        {
            // Standard updates for other particle types
            // Update position based on velocity
            particle.Position += particle.Velocity * delta_s;

            // Standard alpha fade based on lifetime
            float life_ratio = particle.Life / particle.MaxLife;
            particle.Color.a = life_ratio;

            // Update motion component for rendering
            if (registry.motions.has(entity))
            {
                Motion &motion = registry.motions.get(entity);
                motion.position = particle.Position;
                motion.scale = vec2(10.0f * life_ratio);
            }

            // Additional type-specific updates
            if (particle_type == "fire")
            {
                // Fire particles grow slightly as they rise and fade
                if (registry.motions.has(entity))
                {
                    registry.motions.get(entity).scale = vec2(10.0f + 5.0f * (1.0f - life_ratio));
                }

                // Slow down as they rise
                particle.Velocity *= (0.97f);

                // Transition color from orange to yellow to gray smoke
                if (life_ratio < 0.3f)
                {
                    // Fade to gray smoke at end of life
                    particle.Color.r = life_ratio * 2.0f + 0.4f;
                    particle.Color.g = life_ratio * 1.5f + 0.4f;
                    particle.Color.b = life_ratio + 0.4f;
                }
            }
            else if (particle_type == "level_up")
            {
                // Pulsing effect for level-up particles
                float pulse = (sin(particle.Life * 8.0f) * 0.2f) + 0.8f;
                if (registry.motions.has(entity))
                {
                    registry.motions.get(entity).scale = vec2(10.0f * life_ratio * pulse);
                }

                // Increase brightness at pulse peaks
                if (pulse > 0.95f)
                {
                    particle.Color.r = 1.0f;
                    particle.Color.g = 1.0f;
                    particle.Color.b = 0.5f;
                }
            }
        }

        // Update color component if it exists
        if (registry.colors.has(entity))
        {
            registry.colors.get(entity) = vec3(particle.Color);
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
    else if (generator.type == "heal" || generator.type == "poison" || generator.type == "slow")
    {
        // Get sprite dimensions from the generator entity
        vec2 sprite_size = { 50.0f, 50.0f }; // Default size in case motion isn't available

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

        // Heal particles fall upward with slight horizontal variation
        float angle = randomFloat(M_PI / 2 - 0.3f, M_PI / 2 + 0.3f); // Mostly upward
        float speed = randomFloat(30.0f, -100.0f);
        particle.Velocity = { cos(angle) * speed, sin(angle) * speed };

        vec4 colorAdd = vec4(0, 0, 0, 0);
        if (generator.type == "heal")
            colorAdd = vec4(0, randomFloat(0.7f, 1.0f), 0, 0);
        else if (generator.type == "poison")
            colorAdd = vec4(randomFloat(0.3f, 0.6f), 0, randomFloat(0.3f, 0.6f), 0);
        else if (generator.type == "slow")
            colorAdd = vec4(0.3f, 0.3f, randomFloat(0.3f, 0.4f), 0);
        particle.Color = vec4(0, 0, 0, randomFloat(0.7f, 1.0f)) + colorAdd;

        // Varying lifetimes for more natural effect
        particle.MaxLife = randomFloat(0.6f, 1.2f);
    }
    // In createParticle function, modify the electricity_line case:
    else if (generator.type == "electricity_line")
    {
        // Get start and end points
        vec2 start_pos = position;
        vec2 end_pos = position; // Default initialization

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

        // OPTIMIZATION: Skip if too far from the player
        if (registry.players.entities.size() > 0)
        {
            Entity player = registry.players.entities[0];
            float dist_to_player = length(registry.motions.get(player).position - position);
            if (dist_to_player > generator.max_visible_distance)
            {
                return entity; // Early return, don't create particle if too far
            }
        }

        // Find the end position (optimization: only when needed)
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

        // Find which segment this particle belongs to - fewer segments for better performance
        const int NUM_MAIN_SEGMENTS = 15; // Reduced from 30
        int segment_id = registry.particleGenerators.get(generator_entity).particles.size() % NUM_MAIN_SEGMENTS;
        float t = segment_id / (float)NUM_MAIN_SEGMENTS;

        // OPTIMIZATION: Use pre-computed curve control points from ElectricityData
        vec2 p0 = start_pos;
        vec2 p3 = end_pos;
        vec2 p1, p2;

        if (registry.customData.has(generator_entity))
        {
            ElectricityData &elec_data = registry.customData.get(generator_entity);
            p1 = elec_data.curve_ctrl1;
            p2 = elec_data.curve_ctrl2;

            // Add time-based variation so it's not static
            float time_factor = generator.timer / 200.0f;
            p1 += perpendicular * sin(time_factor * 5.0f) * 10.0f;
            p2 += perpendicular * sin(time_factor * 7.0f + 1.3f) * 10.0f;
        }
        else
        {
            // Fallback if no pre-computed data
            p1 = start_pos + path * 0.33f + perpendicular * (sin(t * 10.0f) * 20.0f);
            p2 = start_pos + path * 0.66f + perpendicular * (sin(t * 15.0f + 1.5f) * 20.0f);
        }

        // OPTIMIZATION: Simplified bezier calculation
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        // Calculate position on the bezier curve
        particle.Position = uuu * p0 + 3 * uu * t * p1 + 3 * u * tt * p2 + ttt * p3;

        // Add small jitter for natural look - but less computationally intensive
        float jitter = sin(t * 25.0f + registry.particleGenerators.get(generator_entity).timer / 100.0f) * 8.0f;
        particle.Position += perpendicular * jitter;

        // Calculate tangent in a simplified way
        vec2 tangent = normalize(p3 - p0); // Simplified tangent calculation

        // OPTIMIZATION: Only create branches occasionally with fewer segments
        if (randomFloat(0.0f, 1.0f) < 0.02f && segment_id > 2 && segment_id < NUM_MAIN_SEGMENTS - 2)
        {
            // Branch direction
            float branch_angle = randomFloat(M_PI / 4, 3 * M_PI / 4);
            if (randomFloat(0, 1) > 0.5f)
                branch_angle = -branch_angle;

            vec2 branch_dir = vec2(cos(branch_angle) * tangent.x - sin(branch_angle) * tangent.y,
                                   sin(branch_angle) * tangent.x + cos(branch_angle) * tangent.y);

            // Branch length - shorter for less computation
            float branch_length = randomFloat(15.0f, 40.0f);

            // OPTIMIZATION: Create fewer branch particles
            const int BRANCH_SEGMENTS = 3; // Reduced from 5-8

            for (int i = 0; i < BRANCH_SEGMENTS; i++)
            {
                float branch_t = (i + 1) / (float)BRANCH_SEGMENTS;
                vec2 branch_pos = particle.Position + branch_dir * branch_t * branch_length;

                // Create branch particle
                Entity branch_particle = Entity();
                Particle &b_particle = registry.particles.emplace(branch_particle);
                b_particle.Position = branch_pos;

                // Simple color calculation for branches
                float b_brightness = 0.8f;
                b_particle.Color = {
                    0.3f * b_brightness,
                    0.7f * b_brightness,
                    1.0f * b_brightness,
                    0.8f};

                // Simple velocity
                b_particle.Velocity = branch_dir * 50.0f;

                // Short life for branches
                b_particle.Life = randomFloat(0.08f, 0.15f);
                b_particle.MaxLife = b_particle.Life;

                // Add motion for rendering
                Motion &b_motion = registry.motions.emplace(branch_particle);
                b_motion.position = b_particle.Position;
                b_motion.scale = vec2(15.0f); // Slightly larger for better visibility with fewer particles
                b_motion.angle = atan2(branch_dir.y, branch_dir.x) * 180.0f / M_PI;

                // Add render request
                RenderRequest render_request;
                render_request.used_texture = TEXTURE_ASSET_ID::PARTICLE;
                render_request.used_effect = EFFECT_ASSET_ID::PARTICLE;
                render_request.used_geometry = GEOMETRY_BUFFER_ID::SPRITE;
                registry.renderRequests.insert(branch_particle, render_request);

                // Add to generator's particles list
                ParticleGenerator &actual_generator = registry.particleGenerators.get(generator_entity);
                actual_generator.particles.push_back(branch_particle);
            }
        }

        // Electric blue color - simplified calculation
        float brightness = 0.9f;
        particle.Color = {
            0.3f * brightness, // Less red for more blue look
            0.7f * brightness, // Medium green
            1.0f * brightness, // Full blue
            1.0f               // Full opacity
        };

        // Simple velocity calculation
        particle.Velocity = tangent * 50.0f;

        // Longer life to reduce recreation frequency
        particle.Life = randomFloat(0.15f, 0.25f);
        particle.MaxLife = particle.Life;

        // Set up motion component for rendering with fewer updates
        if (registry.motions.has(entity))
        {
            Motion &motion = registry.motions.get(entity);
            motion.position = particle.Position;
            motion.scale = vec2(20.0f, 8.0f); // Wider for better connectivity with fewer particles
            motion.angle = atan2(tangent.y, tangent.x) * 180.0f / M_PI;
        }
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

Entity ParticleSystem::createAOEEffect(vec2 position, vec2 sprite_size, int duration, Entity target, std::string type)
{
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = sprite_size;

    ParticleGenerator& generator = registry.particleGenerators.emplace(entity);
    generator.type = type;
    generator.amount = 15;
    generator.spawnInterval = 0.1f;
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = duration;
    if (target != NULL) generator.follow_entity = target;

    return entity;
}

Entity ParticleSystem::createElectricityEffect(vec2 start_point, vec2 end_point, float width, float duration_ms)
{
    Entity controller = Entity();

    // Add motion component for the controller entity
    Motion &motion = registry.motions.emplace(controller);
    motion.position = start_point;
    motion.scale = vec2(length(end_point - start_point), width);
    motion.angle = atan2(end_point.y - start_point.y, end_point.x - start_point.x) * 180.0f / M_PI;

    // Add generator component with OPTIMIZED particle count
    ParticleGenerator &generator = registry.particleGenerators.emplace(controller);
    generator.type = "electricity_line";
    generator.amount = 100;           // Reduced from previous values for better performance
    generator.spawnInterval = 0.002f; // Slightly slower spawning to reduce CPU load
    generator.timer = 0.0f;
    generator.isActive = true;
    generator.duration_ms = duration_ms;

    // Store distance threshold for culling
    generator.max_visible_distance = 800.0f; // Don't process if too far away

    // Store the endpoint in a duplicate motion component for reference
    Motion endpoint_motion;
    endpoint_motion.position = end_point;
    registry.motions.emplace_with_duplicates(controller, endpoint_motion);

    // Pre-generate curve control points to avoid recalculating for each particle
    vec2 path = end_point - start_point;
    float path_length = length(path);
    vec2 path_dir = normalize(path);
    vec2 perpendicular = vec2(-path_dir.y, path_dir.x);

    // Store curve data for later use
    ElectricityData &elec_data = registry.customData.emplace(controller);
    elec_data.noise_seed = (float)rand() / RAND_MAX * 1000.0f;
    elec_data.curve_ctrl1 = start_point + path * 0.33f + perpendicular * (sin(elec_data.noise_seed) * 30.0f);
    elec_data.curve_ctrl2 = start_point + path * 0.66f + perpendicular * (sin(elec_data.noise_seed * 1.5f) * 30.0f);

    return controller;
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