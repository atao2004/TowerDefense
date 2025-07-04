#include "common.hpp"
#include "tower_system.hpp"
#include "animation_system.hpp"
#include "particle_system.hpp"
#include <iostream>
#include <algorithm>

TowerSystem::TowerSystem()
{
}

TowerSystem::~TowerSystem()
{
}

void TowerSystem::step(float elapsed_ms)
{
    for (int i = 0; i < registry.towers.entities.size(); i++)
    {
        Tower &tower = registry.towers.components[i];
        Entity entity = registry.towers.entities[i];
        PlantAnimation &plant_anim = registry.plantAnimations.get(entity);
        tower.timer_ms -= elapsed_ms;
        switch (tower.type)
        {
        case PLANT_TYPE::PROJECTILE:
            if (!tower.state)
            {
                Entity target;
                if (find_nearest_enemy(entity, target))
                {
                    tower.state = true;
                    AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.size, false, false, false);
                }
            }
            else
            {
                if (!registry.animations.has(entity))
                {
                    Entity target;
                    if (find_nearest_enemy(entity, target))
                        fire_projectile(entity, target);
                    tower.state = false;
                    AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size, true, false, false);
                }
            }
            break;
        case PLANT_TYPE::HEAL:
            if (!tower.state)
            {
                Entity player = registry.players.entities[0];
                if (compute_delta_distance(entity, player) < tower.range)
                {
                    tower.state = true;
                    AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.size, true, false, false);
                }
            }
            else
            {
                if (tower.timer_ms <= 0)
                {
                    Entity player = registry.players.entities[0];
                    if (compute_delta_distance(entity, player) < tower.range)
                    {
                        Player &player_component = registry.players.components[0];
                        player_component.health = std::min(player_component.health_max, player_component.health + tower.damage);
                        Motion &player_motion = registry.motions.get(player);
                        ParticleSystem::createAOEEffect(player_motion.position, player_motion.scale, PLANT_STATS_MAP.at(plant_anim.id).cooldown, player, "heal");
                    }
                    else
                    {
                        tower.state = false;
                        AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size, true, false, false);
                    }
                    tower.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown;
                }
            }
            break;
        case PLANT_TYPE::POISON:
        case PLANT_TYPE::SLOW:
        {
            if (!tower.state)
            {
                for (uint i = 0; i < registry.enemies.size(); i++)
                {
                    Entity enemy = registry.enemies.entities[i];
                    if (compute_delta_distance(entity, enemy) < tower.range)
                    {
                        tower.state = true;
                        AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.size, true, false, false);
                        break;
                    }
                }
            }
            else
            {
                if (tower.timer_ms <= 0)
                {
                    bool enemy_detected = false;
                    for (uint i = 0; i < registry.enemies.size(); i++)
                    {
                        Entity enemy = registry.enemies.entities[i];
                        if (compute_delta_distance(entity, enemy) < tower.range)
                        {
                            enemy_detected = true;
                            Enemy &enemy_component = registry.enemies.components[i];
                            Motion &enemy_motion = registry.motions.get(enemy);
                            if (tower.type == PLANT_TYPE::POISON)
                            {
                                enemy_component.health -= tower.damage;
                                ParticleSystem::createAOEEffect(enemy_motion.position, enemy_motion.scale, PLANT_STATS_MAP.at(plant_anim.id).cooldown, enemy, "poison");
                            }
                            else if (tower.type == PLANT_TYPE::SLOW)
                            {
                                if (!registry.slowEffects.has(enemy))
                                    registry.slowEffects.emplace(enemy);
                                Slow &slow = registry.slowEffects.get(enemy);
                                slow.value = 1.0f - tower.damage / 100.0f;
                                slow.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown + 100;
                                ParticleSystem::createAOEEffect(enemy_motion.position, enemy_motion.scale, PLANT_STATS_MAP.at(plant_anim.id).cooldown, enemy, "slow");
                            }
                        }
                    }
                    if (!enemy_detected)
                    {
                        tower.state = false;
                        AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size, true, false, false);
                    }
                    tower.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown;
                }
            }
            break;
        }

        case PLANT_TYPE::ELECTRICITY:
        {
            if (!tower.state)
            {
                // Find other electricity towers within range
                std::vector<Entity> nearby_electricity_towers;
                for (int j = 0; j < registry.towers.entities.size(); j++)
                {
                    if (i == j)
                        continue; // Skip self

                    Entity other_tower = registry.towers.entities[j];
                    Tower &other_tower_comp = registry.towers.components[j];

                    // Check if it's an electricity tower
                    if (other_tower_comp.type == PLANT_TYPE::ELECTRICITY)
                    {
                        // Check if within range
                        float distance = compute_delta_distance(entity, other_tower);
                        if (distance < tower.range)
                        {
                            nearby_electricity_towers.push_back(other_tower);
                        }
                    }
                }

                // If there are other electricity towers nearby and cooldown is ready
                if (!nearby_electricity_towers.empty() && tower.timer_ms <= 0)
                {
                    tower.state = true;
                    // Start attack animation
                    AnimationSystem::update_animation(entity,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).attack.duration,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).attack.textures,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).attack.size,
                                                      false, false, false);
                }
            }
            else // In attack state
            {
                // If animation is complete (no animation component means it finished)
                if (!registry.animations.has(entity))
                {
                    // Find nearby electricity towers again
                    std::vector<Entity> nearby_electricity_towers;
                    for (int j = 0; j < registry.towers.entities.size(); j++)
                    {
                        if (i == j)
                            continue;

                        Entity other_tower = registry.towers.entities[j];
                        Tower &other_tower_comp = registry.towers.components[j];

                        if (other_tower_comp.type == PLANT_TYPE::ELECTRICITY &&
                            compute_delta_distance(entity, other_tower) < tower.range)
                        {
                            nearby_electricity_towers.push_back(other_tower);
                        }
                    }

                    // Create electricity between this tower and all nearby electricity towers
                    for (Entity &other_tower : nearby_electricity_towers)
                    {
                        create_electricity_effect(entity, other_tower);
                    }

                    // Reset state and cooldown
                    tower.state = false;
                    tower.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown;

                    // Return to idle animation
                    AnimationSystem::update_animation(entity,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures,
                                                      PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size,
                                                      true, false, false);
                }
            }
            break;
        }
        }
    }
}

void TowerSystem::fire_projectile(Entity tower, Entity target)
{
    if (!registry.towers.has(tower) || !registry.motions.has(tower) ||
        !registry.motions.has(target))
    {
        return;
    }

    Tower &tower_comp = registry.towers.get(tower);
    Motion &tower_motion = registry.motions.get(tower);
    Motion &target_motion = registry.motions.get(target);

    // Create projectile
    Entity projectile = Entity();

    // Add components
    Projectile &proj = registry.projectiles.emplace(projectile);
    proj.source = tower;
    proj.damage = tower_comp.damage;
    proj.speed = PROJECTILE_SPEED;

    Motion &proj_motion = registry.motions.emplace(projectile);
    proj_motion.position = tower_motion.position;
    proj_motion.scale = vec2(10, 10);

    vec2 direction = target_motion.position - tower_motion.position;
    float length = sqrt(dot(direction, direction));
    if (length > 0)
    {
        direction = direction / length;
    }

    proj_motion.velocity = direction * proj.speed;

    PlantAnimation &plant_anim = registry.plantAnimations.get(tower);
    registry.renderRequests.insert(
        projectile,
        {PLANT_PROJECTILE_MAP.at(plant_anim.id),
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});
}

float TowerSystem::compute_delta_distance(Entity tower, Entity target)
{
    Motion &motion_tower = registry.motions.get(tower);
    Motion &motion_target = registry.motions.get(target);
    return std::sqrt(std::pow(motion_tower.position.x - motion_target.position.x, 2) + std::pow(motion_tower.position.y - motion_target.position.y, 2));
}

bool TowerSystem::find_nearest_enemy(Entity entity, Entity &target)
{
    if (registry.towers.has(entity) && registry.motions.has(entity))
    {
        Tower &tower = registry.towers.get(entity);
        Motion &motion = registry.motions.get(entity);

        float min_dist = tower.range;

        for (Entity enemy : registry.enemies.entities)
        {
            if (!registry.motions.has(enemy))
            {
                continue;
            }

            Motion &enemy_motion = registry.motions.get(enemy);
            vec2 diff = enemy_motion.position - motion.position;
            float dist = sqrt(dot(diff, diff));

            if (dist < min_dist)
            {
                min_dist = dist;
                target = enemy;
                return true;
            }
        }
    }

    return false;
}

void TowerSystem::create_electricity_effect(Entity tower1, Entity tower2)
{
    if (!registry.motions.has(tower1) || !registry.motions.has(tower2))
        return;

    // Get tower positions
    Motion &motion1 = registry.motions.get(tower1);
    Motion &motion2 = registry.motions.get(tower2);

    // Create electricity visual effect - duration of 500ms for a quick discharge
    Entity effect = ParticleSystem::createElectricityEffect(motion1.position, motion2.position, 20.0f, 500.0f);

    // Get tower damage
    Tower &tower_comp = registry.towers.get(tower1);

    // Calculate rectangle area between the two towers
    vec2 direction = motion2.position - motion1.position;
    float length = sqrt(dot(direction, direction));
    vec2 normalized_dir = direction / length;
    vec2 perpendicular(-normalized_dir.y, normalized_dir.x);

    // Width of damage area (perpendicular to line connecting towers)
    float damage_width = 40.0f;

    // Check and damage all enemies in the rectangular area
    for (uint i = 0; i < registry.enemies.size(); i++)
    {
        Entity enemy = registry.enemies.entities[i];
        if (!registry.motions.has(enemy))
            continue;

        Motion &enemy_motion = registry.motions.get(enemy);
        Enemy &enemy_comp = registry.enemies.components[i];

        // Calculate if enemy is within the rectangle
        vec2 enemy_relative = enemy_motion.position - motion1.position;
        float along_line = dot(enemy_relative, normalized_dir);
        float perp_dist = abs(dot(enemy_relative, perpendicular));

        // Check if within rectangle bounds
        if (along_line >= 0 && along_line <= length && perp_dist <= damage_width / 2)
        {
            // Damage enemy
            enemy_comp.health -= tower_comp.damage;

            // Add hit effect
            registry.hitEffects.emplace_with_duplicates(enemy);

            // No need for an additional electric effect on the enemy,
            // as the main electricity visual already covers the area
        }
    }
}