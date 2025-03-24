#pragma once

#include "common.hpp"
#include "tinyECS/registry.hpp"
#include "render_system.hpp"

class SeedSystem {
public:
    static void init(RenderSystem* renderer);
    void step(float elapsed_ms);
private:
    static RenderSystem* renderer;
};
