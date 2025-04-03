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
    update_orcriders(elapsed_ms);
    update_squads(elapsed_ms);
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
    Enemy &enemy = registry.enemies.get(entity);
    float current_speed = enemy.speed * 1000;

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
            skeleton_motion.velocity = normalize(direction) * (float)SKELETON_ARCHER_SPEED;
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

void AISystem::update_orcriders(float elapsed_ms)
{
    // Skip if no player exists
    if (registry.players.entities.empty())
    {
        return;
    }

    Entity player = registry.players.entities[0];
    if (!registry.motions.has(player))
    {
        return;
    }

    vec2 player_pos = registry.motions.get(player).position;

    for (auto entity : registry.orcRiders.entities)
    {
        if (!registry.motions.has(entity))
        {
            continue;
        }

        OrcRider &orcrider = registry.orcRiders.get(entity);
        Motion &motion = registry.motions.get(entity);

        // Previous state for detecting state changes
        OrcRider::State prev_state = orcrider.current_state;

        // Calculate distance to player
        vec2 direction = player_pos - motion.position;
        float dist = length(direction);

        // Handle charging state first (has priority)
        if (orcrider.is_charging)
        {
            // Continue charge for the specified distance or until hitting something
            orcrider.charge_timer_ms += elapsed_ms;
            float charge_duration_ms = (orcrider.charge_distance / orcrider.charge_speed) * 1000.0f;

            // Move in charge direction
            motion.velocity = orcrider.charge_direction * orcrider.charge_speed;

            // Check for player collision during charge
            if (!orcrider.has_hit_player && dist < 50.0f)
            {
                // Player was hit by charge
                orcrider.has_hit_player = true;

                // Apply damage to player
                if (registry.statuses.has(player))
                {
                    Status attack_status{
                        "attack",
                        0.0f,
                        static_cast<float>(orcrider.damage)};
                    registry.statuses.get(player).active_statuses.push_back(attack_status);

                    // Play hit sound
                    Mix_PlayChannel(2, injured_sound, 0);
                }
            }

            // When charge is complete
            if (orcrider.charge_timer_ms >= charge_duration_ms)
            {
                orcrider.is_charging = false;
                orcrider.has_hit_player = false;
                motion.velocity = {0, 0};

                // Set a small cooldown to prevent instant hunting
                orcrider.hunt_timer_ms = 300.0f; // Very short cooldown just for animation transition

                // Go back to idle state shortly
                orcrider.current_state = OrcRider::State::IDLE;

                // Play idle animation shortly
                AnimationSystem::update_animation(
                    entity,
                    ORCRIDER_IDLE_ANIMATION_DURATION,
                    ORCRIDER_IDLE_ANIMATION,
                    ORCRIDER_IDLE_ANIMATION_SIZE,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
            }
        }
        // Handle hunting state (preparing to charge)
        else if (orcrider.is_hunting)
        {
            // Keep orc still during hunt animation
            motion.velocity = {0, 0};

            // Update hunt timer
            orcrider.hunt_timer_ms -= elapsed_ms;

            // When hunt animation finishes, start charging
            if (orcrider.hunt_timer_ms <= 0)
            {
                orcrider.is_hunting = false;
                orcrider.is_charging = true;
                orcrider.charge_timer_ms = 0;

                // Calculate and store charge direction
                orcrider.charge_direction = normalize(direction);

                // Update animation to walking for charge
                AnimationSystem::update_animation(
                    entity,
                    ORCRIDER_WALK_ANIMATION_DURATION,
                    ORCRIDER_WALK_ANIMATION,
                    ORCRIDER_WALK_ANIMATION_SIZE,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
            }
        }
        // Regular state management when not hunting/charging
        else
        {
            // Simple cooldown without retreat
            if (orcrider.hunt_timer_ms > 0)
            {
                // Just count down the timer without moving
                orcrider.hunt_timer_ms -= elapsed_ms;
                motion.velocity = {0, 0};
            }
            // Check if in hunt range
            else if (dist <= orcrider.hunt_range)
            {
                orcrider.current_state = OrcRider::State::HUNT;

                // Begin hunt animation if not already hunting
                if (!orcrider.is_hunting)
                {
                    // Start hunt animation
                    orcrider.is_hunting = true;
                    orcrider.hunt_timer_ms = ORCRIDER_HUNT_ANIMATION_DURATION;

                    // Play hunt animation
                    AnimationSystem::update_animation(
                        entity,
                        ORCRIDER_HUNT_ANIMATION_DURATION,
                        ORCRIDER_HUNT_ANIMATION,
                        ORCRIDER_HUNT_ANIMATION_SIZE,
                        false, // don't loop
                        false, // not locked
                        false  // don't destroy
                    );
                }
            }
            // If not in hunt range but within detection range, move towards player
            else if (dist <= orcrider.detection_range)
            {
                orcrider.current_state = OrcRider::State::WALK;

                // Move towards player
                vec2 normalized_direction = normalize(direction);
                motion.velocity = normalized_direction * orcrider.walk_speed;

                // Update facing direction
                if (direction.x != 0)
                {
                    motion.scale.x = (direction.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
                }
            }
            // If too far away, idle
            else
            {
                orcrider.current_state = OrcRider::State::IDLE;
                motion.velocity = {0, 0};
            }
        }

        // Handle animation changes when state changes
        if (!orcrider.is_hunting && !orcrider.is_charging && prev_state != orcrider.current_state)
        {
            switch (orcrider.current_state)
            {
            case OrcRider::State::IDLE:
                // Apply idle animation
                AnimationSystem::update_animation(
                    entity,
                    ORCRIDER_IDLE_ANIMATION_DURATION,
                    ORCRIDER_IDLE_ANIMATION,
                    ORCRIDER_IDLE_ANIMATION_SIZE,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
                break;

            case OrcRider::State::WALK:
                // Apply walk animation
                AnimationSystem::update_animation(
                    entity,
                    ORCRIDER_WALK_ANIMATION_DURATION,
                    ORCRIDER_WALK_ANIMATION,
                    ORCRIDER_WALK_ANIMATION_SIZE,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
                break;

            default:
                break;
            }
        }
    }
}

void AISystem::update_squads(float elapsed_ms)
{
    // Skip if no player exists
    if (registry.players.entities.empty())
        return;

    Entity player = registry.players.entities[0];
    if (!registry.motions.has(player))
        return;

    // Process each squad
    for (auto &squad_entity : registry.squads.entities)
    {
        Squad &squad = registry.squads.get(squad_entity);

        if (!squad.is_active)
            continue;

        // Clean up fallen squad members
        for (auto it = squad.archers.begin(); it != squad.archers.end();)
        {
            if (!registry.motions.has(*it))
                it = squad.archers.erase(it);
            else
                ++it;
        }

        for (auto it = squad.orcs.begin(); it != squad.orcs.end();)
        {
            if (!registry.motions.has(*it))
                it = squad.orcs.erase(it);
            else
                ++it;
        }

        for (auto it = squad.knights.begin(); it != squad.knights.end();)
        {
            if (!registry.motions.has(*it))
                it = squad.knights.erase(it);
            else
                ++it;
        }

        // If all squad members are dead, deactivate squad
        if (squad.archers.empty() && squad.orcs.empty() && squad.knights.empty())
        {
            squad.is_active = false;
            continue;
        }

        // Update the circular formation of archers surrounding the player
        update_archer_circle_formation(squad, elapsed_ms, player);

        // Update orcs to protect their assigned archers
        update_orc_protection(squad, elapsed_ms, player);

        // Update the knight to force player away from archers
        update_knight_herding(squad, elapsed_ms, player);
    }
}

void AISystem::update_archer_circle_formation(Squad &squad, float elapsed_ms, Entity player)
{
    vec2 player_pos = registry.motions.get(player).position;

    // Store the previous player position if not yet stored
    if (length(squad.last_player_pos) < 0.1f)
    {
        squad.last_player_pos = player_pos;
        // Force initial positioning when squad is first created
        squad.coordination_timer = 0.f; // Reset coordination timer
    }

    // Calculate how much the player has moved since last position check
    float player_movement = length(player_pos - squad.last_player_pos);

    // Update coordination timer
    squad.coordination_timer += elapsed_ms;

    // Reposition under three conditions:
    // 1. Player moved significantly
    // 2. Initial positioning (first 3 seconds)
    // 3. Periodic repositioning (every 8 seconds)
    const float REPOSITION_THRESHOLD = 150.0f;
    const float INITIAL_POSITIONING_TIME = 3000.0f; // 3 seconds
    const float PERIODIC_REPOSITION_TIME = 8000.0f; // 8 seconds

    bool should_reposition = player_movement > REPOSITION_THRESHOLD ||
                             squad.coordination_timer < INITIAL_POSITIONING_TIME ||
                             (squad.coordination_timer >= PERIODIC_REPOSITION_TIME &&
                              fmod(squad.coordination_timer, PERIODIC_REPOSITION_TIME) < elapsed_ms);

    // If we're repositioning, update the recorded position
    if (should_reposition)
    {
        squad.last_player_pos = player_pos;
    }

    // Optimal distance from player for archers
    const float OPTIMAL_DISTANCE = 400.0f;

    // Process each archer
    for (size_t i = 0; i < squad.archers.size(); i++)
    {
        Entity archer = squad.archers[i];
        if (!registry.motions.has(archer) || !registry.skeletons.has(archer))
            continue;

        Skeleton &skeleton = registry.skeletons.get(archer);
        Motion &motion = registry.motions.get(archer);

        // Previous state for detecting state changes
        Skeleton::State prev_state = skeleton.current_state;

        // Calculate position in circle around player
        // Distribute evenly around a full circle (2π radians)
        float angle = (2.0f * 3.14159f * i) / squad.archers.size();

        // Create a point on the circle at the optimal distance
        vec2 target_pos = player_pos + vec2(cos(angle), sin(angle)) * OPTIMAL_DISTANCE;

        // Current distance to player
        vec2 dir_to_player = player_pos - motion.position;
        float dist_to_player = length(dir_to_player);

        // Distance to target position
        vec2 to_target = target_pos - motion.position;
        float dist_to_target = length(to_target);

        // Update cooldown timer for attacks
        if (skeleton.cooldown_timer_ms > 0)
        {
            skeleton.cooldown_timer_ms -= elapsed_ms;

            if (skeleton.is_attacking)
            {
                skeleton.attack_timer_ms -= elapsed_ms;

                // Create arrow when attack timer reaches the firing point
                if (skeleton.attack_timer_ms <= 300 && !skeleton.arrow_fired)
                {
                    // Face player before firing
                    if (dir_to_player.x != 0)
                        motion.scale.x = (dir_to_player.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);

                    // Calculate arrow spawn position
                    vec2 normalized_dir = normalize(dir_to_player);
                    vec2 arrow_pos = motion.position + normalized_dir * 30.f;

                    // Create arrow
                    createArrow(arrow_pos, dir_to_player, archer);

                    // Mark that arrow was fired
                    skeleton.arrow_fired = true;
                }
            }
        }

        // Reset attack state when cooldown is complete
        if (skeleton.cooldown_timer_ms <= 0 && skeleton.is_attacking)
        {
            skeleton.is_attacking = false;
            skeleton.arrow_fired = false;
        }

        // Determine whether to move or attack
        bool need_to_move = false;
        bool is_far_from_final_position = dist_to_target > 100.0f;

        // Need to move if:
        // 1. Far from formation position OR
        // 2. Player is too close/far AND we should reposition OR
        // 3. Not in attack range while not attacking
        if (is_far_from_final_position ||
            (should_reposition && (dist_to_player < OPTIMAL_DISTANCE - 100.0f ||
                                   dist_to_player > OPTIMAL_DISTANCE + 100.0f)) ||
            (!skeleton.is_attacking && dist_to_player > skeleton.attack_range))
        {
            need_to_move = true;
        }

        // Can't move while attacking
        if (skeleton.is_attacking)
            need_to_move = false;

        // If we need to move, prioritize movement
        if (need_to_move)
        {
            // Use walking animation and move toward target position
            motion.velocity = normalize(to_target) * 100.0f;
            skeleton.current_state = Skeleton::State::WALK;

            // Face movement direction
            if (to_target.x != 0)
                motion.scale.x = (to_target.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
        }
        else
        {
            // In position, stop and handle attack
            motion.velocity = vec2(0, 0);

            // Set target to player
            skeleton.target = player;

            // Face player
            if (dir_to_player.x != 0)
                motion.scale.x = (dir_to_player.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);

            // If cooldown complete and not attacking, start attack
            // ENSURE we're in attack range here
            if (skeleton.cooldown_timer_ms <= 0 && !skeleton.is_attacking &&
                dist_to_player <= skeleton.attack_range)
            {
                skeleton.current_state = Skeleton::State::ATTACK;
                skeleton.is_attacking = true;
                skeleton.cooldown_timer_ms = skeleton.attack_cooldown_ms;
                skeleton.attack_timer_ms = skeleton.attack_cooldown_ms;
                skeleton.arrow_fired = false;

                // Start attack animation
                AnimationSystem::update_animation(
                    archer,
                    SKELETON_ATTACK_DURATION,
                    SKELETON_ATTACK_ANIMATION,
                    SKELETON_ATTACK_FRAMES,
                    false, // don't loop
                    false, // don't lock
                    false  // don't destroy
                );
            }
            else if (!skeleton.is_attacking)
            {
                // Between attacks, use WALK state with zero velocity
                skeleton.current_state = Skeleton::State::WALK;

                // Make sure we have the proper walking animation
                if (!registry.animations.has(archer) ||
                    registry.animations.get(archer).textures != SKELETON_WALK_ANIMATION)
                {
                    AnimationSystem::update_animation(
                        archer,
                        SKELETON_WALK_DURATION,
                        SKELETON_WALK_ANIMATION,
                        SKELETON_WALK_FRAMES,
                        true,  // loop
                        false, // not locked
                        false  // don't destroy
                    );
                }
            }
        }

        // Handle animation changes
        if (prev_state != skeleton.current_state &&
            !(skeleton.current_state == Skeleton::State::ATTACK &&
              skeleton.is_attacking && registry.animations.has(archer)))
        {
            switch (skeleton.current_state)
            {
            case Skeleton::State::IDLE:
                // We should never reach this - convert IDLE to WALK
                skeleton.current_state = Skeleton::State::WALK;
                // Fall through to WALK case

            case Skeleton::State::WALK:
                // Apply walk animation
                AnimationSystem::update_animation(
                    archer,
                    SKELETON_WALK_DURATION,
                    SKELETON_WALK_ANIMATION,
                    SKELETON_WALK_FRAMES,
                    true,  // loop
                    false, // not locked
                    false  // don't destroy
                );
                break;

            case Skeleton::State::ATTACK:
                // Attack animation is handled above
                break;
            }
        }
    }
}

void AISystem::update_orc_protection(Squad &squad, float elapsed_ms, Entity player)
{
    vec2 player_pos = registry.motions.get(player).position;

    // Each orc protects a specific archer (1:1 relationship)
    for (size_t i = 0; i < squad.orcs.size() && i < squad.archers.size(); i++)
    {
        Entity orc = squad.orcs[i];
        Entity archer = squad.archers[i];

        if (!registry.motions.has(orc) || !registry.motions.has(archer))
            continue;

        Motion &orc_motion = registry.motions.get(orc);
        Motion &archer_motion = registry.motions.get(archer);

        // Calculate vector from archer to player
        vec2 archer_to_player = player_pos - archer_motion.position;
        float archer_to_player_dist = length(archer_to_player);

        // Normalized direction from archer to player
        vec2 direction = normalize(archer_to_player);

        // Position orc between archer and player, but closer to archer
        float protection_distance = min(120.0f, archer_to_player_dist * 0.7f);
        vec2 target_pos = archer_motion.position + direction * protection_distance;

        // Move towards target position
        vec2 to_target = target_pos - orc_motion.position;
        float dist_to_target = length(to_target);

        if (dist_to_target > 10.0f)
        {
            // Move to protection position
            orc_motion.velocity = normalize(to_target) * 150.0f;
        }
        else
        {
            // At protection position
            orc_motion.velocity = vec2(0, 0);

            // If player gets too close to the protected archer, charge at player
            float dist_to_player = length(player_pos - orc_motion.position);
            if (dist_to_player < 150.0f)
            {
                orc_motion.velocity = normalize(player_pos - orc_motion.position) * 180.0f;
            }
        }

        // Face player
        if (archer_to_player.x != 0)
            orc_motion.scale.x = (archer_to_player.x > 0) ? abs(orc_motion.scale.x) : -abs(orc_motion.scale.x);
    }
}

void AISystem::update_knight_herding(Squad &squad, float elapsed_ms, Entity player)
{
    if (squad.knights.empty())
        return;

    // Get the first (and only) knight
    Entity knight = squad.knights[0];
    if (!registry.motions.has(knight) || !registry.orcRiders.has(knight))
        return;

    OrcRider &rider = registry.orcRiders.get(knight);
    Motion &motion = registry.motions.get(knight);

    vec2 player_pos = registry.motions.get(player).position;

    // If already hunting or charging, let normal OrcRider behavior handle it
    if (rider.is_hunting || rider.is_charging)
        return;

    // Determine if player is threatening archers
    bool player_threatening = false;
    vec2 closest_archer_pos;
    float closest_distance = 1000000.0f; // Large initial value

    for (auto archer : squad.archers)
    {
        if (!registry.motions.has(archer))
            continue;

        vec2 archer_pos = registry.motions.get(archer).position;
        float dist = length(player_pos - archer_pos);

        if (dist < closest_distance)
        {
            closest_distance = dist;
            closest_archer_pos = archer_pos;
        }

        // Player is threatening if within 250 pixels of any archer
        if (dist < 250.0f)
        {
            player_threatening = true;
            break;
        }
    }

    // If player is threatening archers, position for charge
    if (player_threatening)
    {
        // Calculate vector from player to closest archer
        vec2 player_to_archer = closest_archer_pos - player_pos;
        vec2 normalized_dir = normalize(player_to_archer);

        // Position perpendicular to the player-archer line to force player away
        vec2 perpendicular = vec2(-normalized_dir.y, normalized_dir.x);

        // Pick the side from which to approach (alternate based on time)
        float side = sin(elapsed_ms / 1000.0f) > 0 ? 1.0f : -1.0f;

        // Set flank position to approach from the side
        vec2 flank_pos = player_pos + perpendicular * side * 300.0f;

        // Move toward flank position
        vec2 to_flank = flank_pos - motion.position;
        float dist_to_flank = length(to_flank);

        if (dist_to_flank > 30.0f)
        {
            motion.velocity = normalize(to_flank) * rider.walk_speed;

            // Update facing direction
            if (to_flank.x != 0)
                motion.scale.x = (to_flank.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
        }
        else
        {
            // In position, prepare to hunt and charge
            rider.is_hunting = true;
            rider.hunt_timer_ms = ORCRIDER_HUNT_ANIMATION_DURATION;
            rider.current_state = OrcRider::State::HUNT;

            // Play hunt animation
            AnimationSystem::update_animation(
                knight,
                ORCRIDER_HUNT_ANIMATION_DURATION,
                ORCRIDER_HUNT_ANIMATION,
                ORCRIDER_HUNT_ANIMATION_SIZE,
                false, // don't loop
                false, // not locked
                false  // don't destroy
            );
        }
    }
    else
    {
        // If player isn't threatening archers, patrol between player and archers
        vec2 center_point = vec2(0, 0);
        int valid_archers = 0;

        for (auto archer : squad.archers)
        {
            if (registry.motions.has(archer))
            {
                center_point += registry.motions.get(archer).position;
                valid_archers++;
            }
        }

        if (valid_archers > 0)
        {
            center_point /= valid_archers;

            // Position halfway between player and archer center
            vec2 patrol_pos = (player_pos + center_point) * 0.5f;

            // Move toward patrol position
            vec2 to_patrol = patrol_pos - motion.position;
            float dist_to_patrol = length(to_patrol);

            if (dist_to_patrol > 20.0f)
            {
                motion.velocity = normalize(to_patrol) * rider.walk_speed;

                // Update facing direction
                if (to_patrol.x != 0)
                    motion.scale.x = (to_patrol.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
            }
            else
            {
                // At patrol position, stay still but face player
                motion.velocity = vec2(0, 0);

                vec2 to_player = player_pos - motion.position;
                if (to_player.x != 0)
                    motion.scale.x = (to_player.x > 0) ? abs(motion.scale.x) : -abs(motion.scale.x);
            }
        }
    }
}