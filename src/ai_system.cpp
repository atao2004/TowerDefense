#include <iostream>
#include "ai_system.hpp"
#include "status_system.hpp"
#include "world_init.hpp"
#include "world_system.hpp"

AISystem::AISystem()
{
}

AISystem::~AISystem()
{
    // Destroy music components
    if (injured_sound != nullptr)
        Mix_FreeChunk(injured_sound);
    Mix_CloseAudio();
}

void AISystem::step(float elapsed_ms)
{
    update_enemy_behaviors(elapsed_ms);
}

void AISystem::update_enemy_behaviors(float elapsed_ms)
{
    // Skip if no player exists
    if (registry.players.entities.empty())
    {
        return;
    }

    // Update each zombie
    for (Entity entity : registry.enemies.entities)
    {
        if (registry.motions.has(entity))
        {
            // Update state if needed (for future use)
            // update_enemy_state(entity);

            // For now, just handle basic movement
            update_zombie_movement(entity, elapsed_ms);
            update_enemy_melee_attack(entity, elapsed_ms);
        }
    }
    update_skeletons(elapsed_ms);
}

void AISystem::update_zombie_movement(Entity entity, float elapsed_ms)
{
    // Currently only implements chase behavior
    handle_chase_behavior(entity, elapsed_ms);
}

void AISystem::handle_chase_behavior(Entity entity, float elapsed_ms)

{
    if (!registry.zombies.has(entity))
    {
        return;
    }
    Motion &motion = registry.motions.get(entity);
    vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;

    // Calculate direction to player
    vec2 direction = calculate_direction_to_target(motion.position, player_pos);

    // If entity has hit effect, reduce chase speed
    Enemy& enemy = registry.enemies.get(entity);
    float current_speed = enemy.speed * 100;

    // Add to velocity instead of overwriting
    float step_seconds = elapsed_ms / 1000.f;
    motion.velocity += direction * current_speed * step_seconds;

    // Optional: Add some drag to prevent infinite acceleration
    motion.velocity *= 0.9f; // Dampening factor
    if (!registry.hitEffects.has(entity))
    {
        // Update facing direction based on total velocity
        if (motion.velocity.x < 0 && motion.scale.x > 0)
        {
            motion.scale.x *= -1;
        }
        else if (motion.velocity.x > 0 && motion.scale.x < 0)
        {
            motion.scale.x *= -1;
        }
    }
}

void AISystem::update_enemy_melee_attack(Entity entity, float elapsed_ms)
{
    if (!registry.players.entities.size())
        return;

    Entity player = registry.players.entities[0];
    Attack &attack = registry.attacks.get(entity);
    Motion &enemy_motion = registry.motions.get(entity);
    Motion &player_motion = registry.motions.get(player);

    attack.range = 40.0f; // Set attack range

    // Calculate distance to player
    float distance = calculate_distance_to_target(enemy_motion.position, player_motion.position);

    // If in range and cooldown ready
    if (distance <= attack.range && !registry.cooldowns.has(entity))
    {
        Mix_PlayChannel(2, injured_sound, 0);

        auto &status_comp = registry.statuses.get(player);

        // Add attack status
        Status attack_status{
            "attack",
            0.0f,                             // 0 duration for immediate effect
            static_cast<float>(attack.damage) // Use enemy's attack damage
        };
        status_comp.active_statuses.push_back(attack_status);

        // Reset cooldown
        Cooldown &cooldown = registry.cooldowns.emplace(entity);
        cooldown.timer_ms = COOLDOWN_ENEMY_ATTACK;
    }
}

vec2 AISystem::calculate_direction_to_target(vec2 start_pos, vec2 target_pos)
{
    vec2 direction = target_pos - start_pos;
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);

    if (length > 0)
    {
        direction.x /= length;
        direction.y /= length;
    }

    return direction;
}

float AISystem::calculate_distance_to_target(vec2 start_pos, vec2 target_pos)
{
    vec2 diff = target_pos - start_pos;
    return sqrt(diff.x * diff.x + diff.y * diff.y);
}

bool AISystem::start_and_load_sounds()
{

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    injured_sound = Mix_LoadWAV(audio_path("injured_sound.wav").c_str());

    if (injured_sound == nullptr)
    {
        fprintf(stderr, "Failed to load sounds\n %s\n make sure the data directory is present",
                audio_path("injured_sound.wav").c_str());
        return false;
    }

    return true;
}

void AISystem::update_skeletons(float elapsed_ms)
{
    if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::TUTORIAL)
    {
        // For tutorial mode, process them differently
        for (auto entity : registry.skeletons.entities)
        {
            if (registry.motions.has(entity))
            {
                // Force velocity to zero, but still allow animations
                Motion &skeleton_motion = registry.motions.get(entity);
                skeleton_motion.velocity = vec2(0.0f, 0.0f);

                if (skeleton_motion.scale.x > 0)
                {
                    skeleton_motion.scale.x *= -1; // Flip to face left
                }

                // If skeleton is not already in attack animation and is not attacking
                Skeleton &skeleton = registry.skeletons.get(entity);
                if (!skeleton.is_attacking && skeleton.current_state != Skeleton::State::ATTACK)
                {
                    // Play idle animation
                    if (!registry.animations.has(entity) ||
                        registry.animations.get(entity).textures != SKELETON_IDLE_ANIMATION)
                    {
                        AnimationSystem::update_animation(
                            entity,
                            SKELETON_IDLE_DURATION,
                            SKELETON_IDLE_ANIMATION,
                            SKELETON_IDLE_FRAMES,
                            true,  // loop
                            false, // not locked
                            false  // don't destroy
                        );
                    }
                }
            }
        }
        return; // Skip regular skeleton update for tutorial
    }

    for (auto entity : registry.skeletons.entities)
    {
        if (!registry.motions.has(entity))
        {
            continue;
        }

        Skeleton &skeleton = registry.skeletons.get(entity);
        Motion &skeleton_motion = registry.motions.get(entity);

        // Previous state for detecting state changes
        Skeleton::State prev_state = skeleton.current_state;

        // Update cooldown timer
        if (skeleton.cooldown_timer_ms > 0)
        {
            skeleton.cooldown_timer_ms -= elapsed_ms;

            // Track attack timing with a separate timer for reliability
            if (skeleton.is_attacking)
            {
                skeleton.attack_timer_ms -= elapsed_ms;

                // Create arrow when attack timer reaches the firing point
                // This happens at a specific point during attack animation
                // Typically halfway through the animation
                if (skeleton.attack_timer_ms <= 300 && !skeleton.arrow_fired)
                {
                    // Fire arrow directly here, not waiting for animation end
                    if (registry.motions.has(skeleton.target))
                    {
                        Motion &target_motion = registry.motions.get(skeleton.target);
                        vec2 direction = target_motion.position - skeleton_motion.position;

                        if (direction.x != 0)
                        {
                            skeleton_motion.scale.x = (direction.x > 0) ? abs(skeleton_motion.scale.x) : -abs(skeleton_motion.scale.x);
                        }

                        // Calculate arrow spawn position
                        vec2 normalized_dir = normalize(direction);
                        vec2 arrow_pos = skeleton_motion.position + normalized_dir * 30.f;

                        // Create arrow
                        createArrow(arrow_pos, direction, entity);

                        // Mark that we've fired the arrow for this attack cycle
                        skeleton.arrow_fired = true;

                        // std::cout << "Arrow created from AI system timer" << std::endl;
                    }
                }
            }
        }

        // Reset attack state when cooldown is complete
        if (skeleton.cooldown_timer_ms <= 0 && skeleton.is_attacking)
        {
            skeleton.is_attacking = false;
            skeleton.arrow_fired = false; // Reset arrow fired flag
        }

        // Target search and evaluation
        Entity nearest_tower = Entity();
        float closest_tower_dist = std::numeric_limits<float>::max();

        if (!registry.towers.entities.empty())
        {
            for (auto tower : registry.towers.entities)
            {
                if (!registry.motions.has(tower))
                    continue;

                vec2 tower_pos = registry.motions.get(tower).position;
                float dist = calculate_distance_to_target(skeleton_motion.position, tower_pos);

                if (dist < closest_tower_dist)
                {
                    closest_tower_dist = dist;
                    nearest_tower = tower;
                }
            }

            skeleton.target = nearest_tower;
        }
        else if (!registry.players.entities.empty())
        {
            skeleton.target = registry.players.entities[0];
        }
        else
        {
            // No valid targets, idle behavior
            skeleton_motion.velocity = {0, 0};
            skeleton.current_state = Skeleton::State::IDLE;
            continue;
        }

        // Ensure target still has motion component
        if (!registry.motions.has(skeleton.target))
        {
            skeleton.target = {};
            continue;
        }

        Motion &target_motion = registry.motions.get(skeleton.target);

        // Calculate distance and direaction to target
        vec2 direction = target_motion.position - skeleton_motion.position;
        float dist = length(direction);

        // Determine behavior and state based on distance
        if (dist > skeleton.attack_range)
        {
            // Target out of range, move towards it
            skeleton_motion.velocity = normalize(direction) * SKELETON_SPEED;
            skeleton.current_state = Skeleton::State::WALK;

            // Update facing direction
            if (direction.x != 0)
            {
                skeleton_motion.scale.x = (direction.x > 0) ? abs(skeleton_motion.scale.x) : -abs(skeleton_motion.scale.x);
            }
        }
        else if (dist < skeleton.stop_distance)
        {
            // Stop and attack
            skeleton_motion.velocity = {0, 0};

            // The state should always be ATTACK when within range
            skeleton.current_state = Skeleton::State::ATTACK;

            // If cooldown complete and not currently attacking, fire arrow
            if (skeleton.cooldown_timer_ms <= 0 && !skeleton.is_attacking)
            {
                skeleton.is_attacking = true;
                skeleton.cooldown_timer_ms = skeleton.attack_cooldown_ms;
                skeleton.attack_timer_ms = skeleton.attack_cooldown_ms; // Set attack timer
                skeleton.arrow_fired = false;                           // Reset arrow fired flag

                // Start the attack animation right away
                AnimationSystem::update_animation(
                    entity,
                    SKELETON_ATTACK_DURATION,
                    SKELETON_ATTACK_ANIMATION,
                    SKELETON_ATTACK_FRAMES,
                    false, // don't loop
                    false, // don't lock
                    false  // don't destroy
                );
            }
            // If we're in attack range but between attacks and have no animation,
            // use the walk animation as a fallback (no IDLE state in playing mode)
            else if (!skeleton.is_attacking && !registry.animations.has(entity))
            {
                // Use WALK animation between attacks (not IDLE)
                AnimationSystem::update_animation(
                    entity,
                    SKELETON_WALK_DURATION,
                    SKELETON_WALK_ANIMATION,
                    SKELETON_WALK_FRAMES,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
            }
        }

        // Handle animation changes when state changes
        if (prev_state != skeleton.current_state ||
            (skeleton.current_state == Skeleton::State::ATTACK && skeleton.is_attacking && !registry.animations.has(entity)))
        {
            bool currently_attacking = skeleton.is_attacking &&
                                       registry.animations.has(entity) &&
                                       registry.animations.get(entity).textures == SKELETON_ATTACK_ANIMATION;
            if (!currently_attacking)
            {
                switch (skeleton.current_state)
                {
                case Skeleton::State::IDLE:
                    // Apply idle animation
                    AnimationSystem::update_animation(
                        entity,
                        SKELETON_IDLE_DURATION,
                        SKELETON_IDLE_ANIMATION,
                        SKELETON_IDLE_FRAMES,
                        true,  // loop
                        false, // not locked
                        false  // don't destroy
                    );
                    break;

                case Skeleton::State::WALK:
                    // Apply walk animation
                    AnimationSystem::update_animation(
                        entity,
                        SKELETON_WALK_DURATION,
                        SKELETON_WALK_ANIMATION,
                        SKELETON_WALK_FRAMES,
                        true,  // loop
                        false, // not locked
                        false  // don't destroy
                    );
                    break;

                case Skeleton::State::ATTACK:
                    if (skeleton.is_attacking)
                    {
                        // Apply attack animation when entering attack state and starting to attack
                        AnimationSystem::update_animation(
                            entity,
                            SKELETON_ATTACK_DURATION,
                            SKELETON_ATTACK_ANIMATION,
                            SKELETON_ATTACK_FRAMES,
                            false, // don't loop
                            false, // don't lock animation
                            false  // don't destroy
                        );
                    }
                    break;
                }
            }
        }
    }
}