#include "common.hpp"
#include "tower_system.hpp"
#include "animation_system.hpp"
#include <iostream>

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
        Entity entity = registry.towers.entities[i];
        Tower& tower = registry.towers.components[i];
        if (!tower.state)
        {
            Entity target;
            Motion& motion = registry.motions.get(entity);
            if (find_nearest_enemy(motion.position, tower.range, target))
            {
                fire_projectile(entity, target);
                tower.state = true;
                AnimationSystem::update_animation(entity, PLANT_ATTACK_DURATION, PLANT_ATTACK_ANIMATION, PLANT_ATTACK_SIZE, false, false, false);
            }
            else {}
        }
    }
}

void TowerSystem::handle_tower_attacks(float elapsed_ms)
{
    /*for (Entity entity : registry.towers.entities)
    {
        if (!registry.towers.has(entity) || !registry.motions.has(entity))
        {
            continue;
        }

        Tower &tower = registry.towers.get(entity);
        Motion &tower_motion = registry.motions.get(entity);

        tower.timer_ms -= elapsed_ms;

        if (tower.timer_ms <= 0)
        {
            Entity target = find_nearest_enemy(tower_motion.position, tower.range);

            if (target != Entity())
            {
                fire_projectile(entity, target);
                tower.timer_ms = 1000;
            }
        }
    }*/
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

bool TowerSystem::find_nearest_enemy(vec2 tower_pos, float range, Entity& target)
{
    float min_dist = range;

    for (Entity zombie : registry.zombies.entities)
    {
        if (!registry.motions.has(zombie))
        {
            continue;
        }

        Motion &zombie_motion = registry.motions.get(zombie);
        vec2 diff = zombie_motion.position - tower_pos;
        float dist = sqrt(dot(diff, diff));

        if (dist < min_dist)
        {
            min_dist = dist;
            target = zombie;
            return true;
        }
    }
    
    return false;
}