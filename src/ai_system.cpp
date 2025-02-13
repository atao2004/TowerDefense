#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

void AISystem::step(float elapsed_ms) {
    update_enemy_behaviors(elapsed_ms);
}

void AISystem::update_enemy_behaviors(float elapsed_ms) {
    // Skip if no player exists
    if (registry.players.entities.empty()) {
        return;
    }

    // Update each enemy
    for (Entity entity : registry.zombies.entities) {
        if (registry.motions.has(entity)) {
            // Update state if needed (for future use)
            // update_enemy_state(entity);
            
            // For now, just handle basic movement
            update_enemy_movement(entity, elapsed_ms);
        }
    }
}

void AISystem::update_enemy_movement(Entity entity, float elapsed_ms) {
    // Currently only implements chase behavior
    handle_chase_behavior(entity, elapsed_ms);
}

void AISystem::handle_chase_behavior(Entity entity, float elapsed_ms) {
    Motion& motion = registry.motions.get(entity);
    vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;
    
    // Calculate direction to player
    vec2 direction = calculate_direction_to_target(motion.position, player_pos);
    
    // Update velocity
    float step_seconds = elapsed_ms / 1000.f;
    motion.velocity = direction * BASE_ENEMY_SPEED;
    
    // Update facing direction
    if (motion.velocity.x < 0 && motion.scale.x > 0) {
        motion.scale.x *= -1;
    } else if (motion.velocity.x > 0 && motion.scale.x < 0) {
        motion.scale.x *= -1;
    }
}

vec2 AISystem::calculate_direction_to_target(vec2 start_pos, vec2 target_pos) {
    vec2 direction = target_pos - start_pos;
    float length = sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length > 0) {
        direction.x /= length;
        direction.y /= length;
    }
    
    return direction;
}

float AISystem::calculate_distance_to_target(vec2 start_pos, vec2 target_pos) {
    vec2 diff = target_pos - start_pos;
    return sqrt(diff.x * diff.x + diff.y * diff.y);
}
