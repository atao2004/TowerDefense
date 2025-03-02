#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "render_system.hpp"

class AnimationSystem {
public:
    static void init(RenderSystem* renderer);
    static void step(float elapsed_ms);
    static void update_animation(Entity entity, int frame_delay, const TEXTURE_ASSET_ID* textures, int textures_size, bool loop, bool lock, bool destroy);
private:
    static RenderSystem* renderer;
    static void handle_animation_end(Entity entity);
};
