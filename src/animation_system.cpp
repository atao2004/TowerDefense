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
            if (request.used_texture == TEXTURE_ASSET_ID::ZOMBIE_WALK_1) {
                request.used_texture = TEXTURE_ASSET_ID::ZOMBIE_WALK_2;
            }
            else {
                request.used_texture = TEXTURE_ASSET_ID::ZOMBIE_WALK_1;
            }
            animation.timer_ms = 0;
        }
    }
}
