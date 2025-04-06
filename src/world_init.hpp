#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"
#include "seeds.hpp"

//splash screen
Entity createScreen(RenderSystem* renderer, TEXTURE_ASSET_ID background);
Entity createButton(RenderSystem* renderer, BUTTON_ID type, vec2 position, vec2 toDeduct);

// enemies
Entity createZombieSpawn(RenderSystem* renderer, vec2 position);
Entity createEnemy(RenderSystem* renderer, vec2 position, int health, int damage, int speed, int anim_duration, const TEXTURE_ASSET_ID* anim_textures, int anim_size);
Entity createOrc(RenderSystem* renderer, vec2 position);
Entity createOrcElite(RenderSystem* renderer, vec2 position);
Entity createSkeleton(RenderSystem* renderer, vec2 position);
Entity createCharacter(RenderSystem* renderer, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture);
Entity createWerebear(RenderSystem* renderer, vec2 position);
Entity createWerewolf(RenderSystem* renderer, vec2 position);
Entity createSlime(RenderSystem* renderer, vec2 position);
Entity createOrcRider(RenderSystem* renderer, vec2 position);

// towers
Entity createPlant(RenderSystem* renderer, vec2 position, PLANT_ID id);

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
Entity createPausePanel(RenderSystem* renderer, vec2 position);

// tutorial components
Entity createTutorialMove(vec2 position);
Entity createTutorialAttack(vec2 position);
Entity createTutorialPlant(vec2 position);
Entity createTutorialRestart(vec2 position);
Entity createTutorialArrow(vec2 position);

// ui elements
Entity createPause(vec2 position);
Entity createToolbar(vec2 position);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 position, int seed_type);
Entity createEffect(RenderSystem* renderer, vec2 position, vec2 scale);
// create seed (for milestone #2)
Entity createSeed(vec2 pos, int type);
Entity createSeedInventory(vec2 pos, vec2 velocity, int type, int toolbar_pos);
Entity createCamera(RenderSystem* renderer, vec2 position);

Entity createGameOver();

Entity createSkeletonArcher(RenderSystem* renderer, vec2 position);
Entity createArrow(vec2 position, vec2 direction, Entity source);

Entity createText(std::string text, vec2 pos, float size, vec3 color);

