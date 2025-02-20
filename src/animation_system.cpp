// In status_system.cpp
#include "animation_system.hpp"
#include <iostream>

void AnimationSystem::step(float elapsed_ms)
{
    for (Entity entity : registry.zombies.entities) {
        Animation& animation = registry.animations.get(entity);
        animation.timer_ms += elapsed_ms;
        if (animation.timer_ms >= ZOMBIE_MOVE_FRAME_DELAY) {
            RenderRequest& request = registry.renderRequests.get(entity);
            animation.pose = (animation.pose + 1) % (sizeof(ZOMBIE_ANIMATION) / sizeof(ZOMBIE_ANIMATION[0]));
            request.used_texture = ZOMBIE_ANIMATION[animation.pose];
            animation.timer_ms = 0;
        }
    }
}
