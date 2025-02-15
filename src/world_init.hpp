#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// invaders
Entity createZombie(RenderSystem* renderer, vec2 position);

// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// ui elements
Entity createHealthbar();
Entity createPause();
Entity createToolbar();
Entity createExpbar();

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 position);
