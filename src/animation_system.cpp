// In status_system.cpp
#include "animation_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include "state_system.hpp"

RenderSystem* AnimationSystem::renderer;

void AnimationSystem::init(RenderSystem* renderer_arg)
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
    for (Entity entity : registry.animations.entities) {
        Animation& animation = registry.animations.get(entity);
        animation.timer_ms += elapsed_ms;
        if (animation.timer_ms >= animation.transition_ms) {
            animation.pose += 1;
            if (!(animation.pose < animation.pose_count)) {
                if (!animation.loop) {
                    handle_animation_end(entity);
                    if (animation.destroy) {
                        registry.remove_all_components_of(entity);
                    }
                    else {
                        registry.animations.remove(entity);
                    }
                    break;
                }
                animation.pose = 0;
            }
            RenderRequest& request = registry.renderRequests.get(entity);
            request.used_texture = animation.textures[animation.pose];
            animation.timer_ms = 0;
        }
    }
}

/**
* Replace the animation component of the entity.
* 
* @param entity The entity.
* @param frame_delay The transition time between each texture.
* @param textures The array of textures.
* @param textures_size The size of textures.
* @param loop True if the animation should loop.
* @param lock True if the animation should not be replaced.
*/
void AnimationSystem::update_animation(Entity entity, int frame_delay, const TEXTURE_ASSET_ID* textures, int textures_size, bool loop, bool lock, bool destroy)
{
    if (registry.animations.has(entity) && registry.animations.get(entity).lock) {
        //
    }
    else {
        if (registry.animations.has(entity)) {
            registry.animations.remove(entity);
        }
        Animation& animation = registry.animations.emplace(entity);
        animation.transition_ms = frame_delay;
        animation.pose_count = textures_size;
        animation.textures = textures;
        animation.loop = loop;
        animation.lock = lock;
        animation.destroy = destroy;
        RenderRequest& request = registry.renderRequests.get(entity);
        request.used_texture = textures[0];
    }
}

void AnimationSystem::handle_animation_end(Entity entity)
{
    Animation& animation = registry.animations.get(entity);
    if (registry.zombieSpawns.has(entity)) {
        Motion& motion = registry.motions.get(entity);
        createZombie(renderer, motion.position);
    }
    if (registry.states.has(entity)) {
        StateSystem::update_state(STATE::IDLE);
    }
}
