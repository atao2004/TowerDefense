// In status_system.cpp
#include "animation_system.hpp"
#include <iostream>

void AnimationSystem::step(float elapsed_ms)
{
    for (Entity entity : registry.animations.entities) {
        Animation& animation = registry.animations.get(entity);
        animation.timer_ms += elapsed_ms;
        if (animation.timer_ms >= animation.transition_ms) {
            animation.pose += 1;
            if (!(animation.pose < animation.pose_count)) {
                if (!animation.loop) {
                    registry.animations.remove(entity);
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
