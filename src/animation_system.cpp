// In status_system.cpp
#include "animation_system.hpp"
#include <iostream>

void AnimationSystem::step(float elapsed_ms)
{
    for (Entity entity : registry.animations.entities) {
        Animation& animation = registry.animations.get(entity);
        animation.timer_ms += elapsed_ms;
        if (animation.timer_ms >= animation.transition_ms) {
            RenderRequest& request = registry.renderRequests.get(entity);
            animation.pose = (animation.pose + 1) % animation.pose_count;
            request.used_texture = animation.textures[animation.pose];
            animation.timer_ms = 0;
        }
    }
}
