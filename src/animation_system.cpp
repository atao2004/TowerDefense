// In status_system.cpp
#include "animation_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include "player_system.hpp"

RenderSystem *AnimationSystem::renderer;

void AnimationSystem::init(RenderSystem *renderer_arg)
{
    renderer = renderer_arg;
}

/**
 * Update the texture of each animation component.
 *
 * @param elapsed_ms The change in time (milliseconds).
 */
void AnimationSystem::step(float elapsed_ms)
{
    for (Entity entity : registry.animations.entities)
    {
        Animation &animation = registry.animations.get(entity);
        animation.runtime_ms += elapsed_ms;
        animation.timer_ms += elapsed_ms;
        if (animation.timer_ms >= animation.transition_ms)
        {
            animation.pose += 1;
            if (!(animation.pose < animation.pose_count))
            {
                if (!animation.loop)
                {
                    handle_animation_end(entity);
                    break;
                }
                animation.pose = 0;
            }
            RenderRequest &request = registry.renderRequests.get(entity);
            if (animation.textures != NULL)
                request.used_texture = animation.textures[animation.pose];
            animation.timer_ms = 0;
        }
    }
}

/**
 * Replace the animation component of the entity.
 *
 * @param entity The entity.
 * @param duration The total duration of the animation.
 * @param textures The array of textures.
 * @param textures_size The size of textures.
 * @param loop True if the animation should loop.
 * @param lock True if the animation should not be replaced.
 * @param destroy True if the entity should be destroyed at the end of the animation.
 */
void AnimationSystem::update_animation(Entity entity, int duration, const TEXTURE_ASSET_ID *textures, int textures_size, bool loop, bool lock, bool destroy)
{
    if (registry.animations.has(entity) && registry.animations.get(entity).lock)
    {
        //
    }
    else
    {
        if (registry.animations.has(entity))
        {
            registry.animations.remove(entity);
        }
        Animation &animation = registry.animations.emplace(entity);
        animation.transition_ms = duration / textures_size;
        animation.pose_count = textures_size;
        animation.textures = textures;
        animation.loop = loop;
        animation.lock = lock;
        animation.destroy = destroy;
        RenderRequest &request = registry.renderRequests.get(entity);
        request.used_texture = textures[0];
    }
}

void AnimationSystem::handle_animation_end(Entity entity)
{
    Animation &animation = registry.animations.get(entity);

    // std::cout << "Animation ended for entity " << entity << std::endl;

    // Handle zombie spawn
    if (registry.zombieSpawns.has(entity))
    {
        Motion &motion = registry.motions.get(entity);
        createZombie(renderer, motion.position);
    }

    // Handle player state change
    if (registry.states.has(entity))
    {
        PlayerSystem::update_state(STATE::IDLE);
    }


    // Handle plant attack end
    if (registry.towers.has(entity))
    {
        Tower& tower = registry.towers.get(entity);
        tower.state = false;
    }

    // If animation was set to destroy entity when done
    if (animation.destroy)
    {
        registry.remove_all_components_of(entity);
    }
    else if (!animation.loop)
    {
        // If not looping and not destroying, remove the animation component
        registry.animations.remove(entity);
    }
}