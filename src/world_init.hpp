#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// zombies
Entity createZombie(RenderSystem* renderer, vec2 position);

// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// texture elements
Entity createGrass(vec2 position);
Entity createFarmland(vec2 position);
Entity createScorchedEarth(vec2 position);
void removeSurfaces();

// ui elements
Entity createPause();
Entity createToolbar();

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 position);
Entity createEffect(RenderSystem* renderer, vec2 position, vec2 scale);
// create seed (for milestone #2)
Entity createSeed(vec2 pos);
