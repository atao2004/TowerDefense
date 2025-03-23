#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// zombies
Entity createZombieSpawn(RenderSystem* renderer, vec2 position);
Entity createOrc(RenderSystem* renderer, vec2 position);

// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// chicken
Entity createChicken(RenderSystem* renderer);

// texture elements
Entity createMapTile(Entity maptile_entity, vec2 position);
Entity createMapTile(vec2 position);
Entity createMapTileDecoration(Entity decoration_entity, int i, vec2 position);
Entity createMapTileDecoration(int i, vec2 position);
Entity createTutorialTile(vec2 position);
Entity createTutorialTileDecoration(int i, vec2 position);
Entity createScorchedEarth(vec2 position);
void removeSurfaces();

// texture elements (new)
void parseMap(bool tutorial);

// tutorial components
Entity createTutorialMove(vec2 position);
Entity createTutorialAttack(vec2 position);
Entity createTutorialPlant(vec2 position);
Entity createTutorialRestart(vec2 position);
Entity createTutorialArrow(vec2 position);

// ui elements
Entity createPause();
Entity createToolbar(vec2 position);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 position);
Entity createEffect(RenderSystem* renderer, vec2 position, vec2 scale);
// create seed (for milestone #2)
Entity createSeed(vec2 pos, int type);
Entity createSeedInventory(vec2 pos, vec2 velocity, int type);
Entity createCamera(RenderSystem* renderer, vec2 position);

Entity createGameOver();

Entity createSkeletonArcher(RenderSystem* renderer, vec2 position);
Entity createArrow(vec2 position, vec2 direction, Entity source);

