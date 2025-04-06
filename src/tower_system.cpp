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
        Tower& tower = registry.towers.components[i];
        Entity entity = registry.towers.entities[i];
        PlantAnimation& plant_anim = registry.plantAnimations.get(entity);
        tower.timer_ms -= elapsed_ms;
        switch (tower.type) {
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
                    if (compute_delta_distance(entity, player) < tower.range) {
                        tower.state = true;
                        AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).attack.size, true, false, false);
                    }
                }
                else
                {
                    if (tower.timer_ms <= 0)
                    {
                        Entity player = registry.players.entities[0];
                        if (compute_delta_distance(entity, player) < tower.range) {
                            Player& player_component = registry.players.components[0];
                            player_component.health = std::min(player_component.health_max, player_component.health + tower.damage);
                        }
                        else {
                            tower.state = false;
                            AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size, true, false, false);
                        }
                        tower.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown;
                    }
                }
                break;
            case PLANT_TYPE::POISON:
            case PLANT_TYPE::SLOW: {
                if (!tower.state)
                {
                    for (uint i = 0; i < registry.enemies.size(); i++) {
                        Entity enemy = registry.enemies.entities[i];
                        if (compute_delta_distance(entity, enemy) < tower.range) {
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
                        for (uint i = 0; i < registry.enemies.size(); i++) {
                            Entity enemy = registry.enemies.entities[i];
                            if (compute_delta_distance(entity, enemy) < tower.range) {
                                enemy_detected = true;
                                Enemy& enemy_component = registry.enemies.components[i];
                                if (tower.type == PLANT_TYPE::POISON)
                                    enemy_component.health -= tower.damage;
                                else if (tower.type == PLANT_TYPE::SLOW) {
                                    if (!registry.slowEffects.has(enemy))
                                        registry.slowEffects.emplace(enemy);
                                    Slow& slow = registry.slowEffects.get(enemy);
                                    slow.value = 1.0f - tower.damage / 100.0f;
                                    slow.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown + 100;
                                }
                            }
                        }
                        if (!enemy_detected) {
                            tower.state = false;
                            AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.duration, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.textures, PLANT_ANIMATION_MAP.at(plant_anim.id).idle.size, true, false, false);
                        }
                        tower.timer_ms = PLANT_STATS_MAP.at(plant_anim.id).cooldown;
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

    registry.renderRequests.insert(
        projectile,
        {TEXTURE_ASSET_ID::PROJECTILE,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});
}

float TowerSystem::compute_delta_distance(Entity tower, Entity target)
{
    Motion& motion_tower = registry.motions.get(tower);
    Motion& motion_target = registry.motions.get(target);
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