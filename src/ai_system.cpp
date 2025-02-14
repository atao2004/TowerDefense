#include <iostream>
#include "ai_system.hpp"
#include "status_system.hpp"
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
			handle_enemy_attack(entity, elapsed_ms);
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

void AISystem::handle_enemy_attack(Entity entity, float elapsed_ms) {
    if (!registry.players.entities.size()) return;
    
    Entity player = registry.players.entities[0];
    Attack& attack = registry.attacks.get(entity);
    Motion& enemy_motion = registry.motions.get(entity);
    Motion& player_motion = registry.motions.get(player);
    
    // Update cooldown
    attack.cooldown_ms -= elapsed_ms;
    
    // Calculate distance to player
    float distance = calculate_distance_to_target(enemy_motion.position, player_motion.position);
    
    // If in range and cooldown ready
    if (distance <= attack.range && attack.cooldown_ms <= 0) {
        
        auto& status_comp = registry.statuses.get(player);
        
        // Add attack status
        Status attack_status{
            "attack",
            0.0f,           // 0 duration for immediate effect
            static_cast<float>(attack.damage)   // Use enemy's attack damage
        };
        status_comp.active_statuses.push_back(attack_status);
        
        
        // Reset cooldown
        attack.cooldown_ms = 1000.0f;  // Reset to 1 second
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
