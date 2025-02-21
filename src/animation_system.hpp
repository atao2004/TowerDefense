#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"

class AnimationSystem {
public:
    static void step(float elapsed_ms);
    static void update_animation(Entity entity, int frame_delay, const TEXTURE_ASSET_ID* textures, int textures_size, bool loop, bool lock);
};
