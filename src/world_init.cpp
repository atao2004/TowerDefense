#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include "animation_system.hpp"
#include "../ext/json.hpp"
using json = nlohmann::json;

Entity createGridLine(vec2 start_pos, vec2 end_pos) {
	Entity entity = Entity();
	GridLine& gl = registry.gridLines.emplace(entity);
	gl.start_pos = start_pos;
	gl.end_pos = end_pos;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	vec3& cv = registry.colors.emplace(entity);
	cv = GRID_COLOR;

	return entity;
}

// createZombieSpawn
Entity createZombieSpawn(RenderSystem* renderer, vec2 position) {
	Entity entity = Entity();

	registry.zombieSpawns.emplace(entity);

	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ ZOMBIE_WIDTH, ZOMBIE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ZOMBIE_SPAWN_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);

	AnimationSystem::update_animation(entity, ZOMBIE_SPAWN_FRAME_DELAY, ZOMBIE_SPAWN_ANIMATION, sizeof(ZOMBIE_SPAWN_ANIMATION) / sizeof(ZOMBIE_SPAWN_ANIMATION[0]), false, false, true);

	return entity;
}

Entity createZombie(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Zombie& zombie = registry.zombies.emplace(entity);
	zombie.health = ZOMBIE_HEALTH;

	Attack& attack = registry.attacks.emplace(entity);
	attack.range = 30.0f;
	attack.damage = ZOMBIE_DAMAGE;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ ZOMBIE_WIDTH, ZOMBIE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ZOMBIE_WALK_1,
			EFFECT_ASSET_ID::ZOMBIE,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	AnimationSystem::update_animation(entity, ZOMBIE_MOVE_FRAME_DELAY, ZOMBIE_ANIMATION, sizeof(ZOMBIE_ANIMATION) / sizeof(ZOMBIE_ANIMATION[0]), true, false, false);

	// Kung: Update the enemy count and print it to the console.
    std::cout << "Enemy count: " << registry.zombies.size() << " zombies" << std::endl;

	return entity;
}

Entity createTower(RenderSystem* renderer, vec2 position) {
    Entity entity = Entity();

    // Basic tower stats
    Tower& tower = registry.towers.emplace(entity);
    tower.health = 100.f;
    tower.damage = 10.f;
    tower.range = 2000.f;     // Detection range in pixels
    tower.timer_ms = 2000;   // Attack every 2 second

    // Motion component for position and rotation
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.f;
    motion.velocity = { 0, 0 };  // Towers don't move
    motion.scale = vec2({ TOWER_BB_WIDTH, TOWER_BB_HEIGHT });  // Using constants from common.hpp

	Dimension& dimension = registry.dimensions.emplace(entity);
	dimension.width = TOWER_BB_WIDTH;
	dimension.height = TOWER_BB_HEIGHT;

	
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

    // Add render request for tower
    registry.renderRequests.insert(
        entity,
        {
            TEXTURE_ASSET_ID::TOWER,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE
        }
    );

    return entity;
}

void removeTower(vec2 position) {
	// remove any towers at this position
	for (Entity& tower_entity : registry.towers.entities) {
		// get each tower's position to determine it's row
		const Motion& tower_motion = registry.motions.get(tower_entity);
		
		if (tower_motion.position.y == position.y) {
			// remove this tower
			registry.remove_all_components_of(tower_entity);
			std::cout << "tower removed" << std::endl;
		}
	}
}

// Kung: Create the grass texture that will be used as part of the texture map.
Entity createMapTile(int i, vec2 position) {
	// Create the associated entity.
	Entity maptile_entity = Entity();

	// Create the associated component.
	MapTile& maptile_component = registry.mapTiles.emplace(maptile_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(maptile_entity);
	motion_component.position = position;
	motion_component.scale = DECORATION_SIZE_LIST[i];
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		maptile_entity,
		{
			DECORATION_LIST[i],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return maptile_entity;
}

Entity createTutorialTile(int i, vec2 position) {
	// Create the associated entity.
	Entity tutorial_tile_entity = Entity();

	// Create the associated component.
	TutorialTile& tutorial_tile_component = registry.tutorialTiles.emplace(tutorial_tile_entity);

	// Create the associated maptile component.
	MapTile& maptile_component = registry.mapTiles.emplace(tutorial_tile_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(tutorial_tile_entity);
	motion_component.position = position;
	motion_component.scale = DECORATION_SIZE_LIST[i];
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		tutorial_tile_entity,
		{
			DECORATION_LIST[i],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return tutorial_tile_entity;

}

Entity createDecoration(int i, vec2 position) {
	// Create the associated entity.
	Entity decoration_entity = Entity();

	// Create the associated maptile component.
	MapTile& maptile_component = registry.mapTiles.emplace(decoration_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(decoration_entity);
	motion_component.position = position;
	motion_component.scale = DECORATION_SIZE_LIST[i];
	motion_component.velocity = vec2(0, 0);
	// Render the object.
	registry.renderRequests.insert(
		decoration_entity,
		{
			DECORATION_LIST[i],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return decoration_entity;
}

// Kung: Create the tiles that border the window and represent areas in which the player cannot walk on.
// Technically, the tiles represent dirt, but will look different from the farmland that will be implemented later.
Entity createScorchedEarth(vec2 position)
{
	// Create the associated entity.
	Entity scorched_earth_entity = Entity();

	// Create the associated component.
	ScorchedEarth& scorched_earth_component = registry.scorchedEarths.emplace(scorched_earth_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(scorched_earth_entity);
	motion_component.position = position;
	motion_component.scale = vec2(SCORCHED_EARTH_DIMENSION_PX, SCORCHED_EARTH_DIMENSION_PX);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		scorched_earth_entity,
		{
			TEXTURE_ASSET_ID::SCORCHED_EARTH,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return scorched_earth_entity;
}

// Kung: Remove all surfaces before adding in new ones when resetting the game.
// This includes grass and scorched earth, and eventually farmland as well.
void removeSurfaces()
{
	// remove all mapTiles
	for (Entity& maptile_entity : registry.mapTiles.entities) {
		registry.remove_all_components_of(maptile_entity);
	}
	// remove all scorched earth
	for (Entity& scorched_earth_entity : registry.scorchedEarths.entities) {
		registry.remove_all_components_of(scorched_earth_entity);
	}
	// print confirmation
	std::cout << "surfaces reset" << std::endl;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialSign(vec2 position, TEXTURE_ASSET_ID asset_id) {
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign& tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(450, 360);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{
			asset_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return tutorial_entity;
}

void parseMap(bool tutorial) {
	json jsonFile;
	if (tutorial) {
		std::ifstream file("data/map/tutorialMap.json");
		file>>jsonFile;
	} else  {
		std::ifstream file("data/map/myMap.json");
		file>>jsonFile;
	}

	int numCol = jsonFile["width"];
	int numRow = jsonFile["height"];
	std::vector<int> map_layer = jsonFile["layers"][0]["data"];
	std::vector<int> decoration_layer = jsonFile["layers"][1]["data"];

	// create background
	for (int i=0; i<numRow; i++) { //iterating row-by-row
		for (int j=0; j<numCol; j++) {
			if (map_layer[i*numCol+j] == 1) {
				if (tutorial) {
					createTutorialTile(1, {j*GRID_CELL_WIDTH_PX, i*GRID_CELL_HEIGHT_PX});
				} else {
					createMapTile(1, {j*GRID_CELL_WIDTH_PX, i*GRID_CELL_HEIGHT_PX});
				}
			}
		}
	}

	// add decorations
	for (int i=0; i<numRow; i++) { //iterating row-by-row
		for (int j=0; j<numCol; j++) {	
			if (decoration_layer[i*numCol+j] != 0) {
				if (tutorial) {
					createTutorialTile(decoration_layer[i*numCol+j], {j*GRID_CELL_WIDTH_PX, i*GRID_CELL_HEIGHT_PX});
				} else {
					createMapTile(decoration_layer[i*numCol+j], {j*GRID_CELL_WIDTH_PX, i*GRID_CELL_HEIGHT_PX});
				}
			}
		}
	}
}

// Kung: Create the toolbar that in the future will store seeds, harvests, and other associated items.
// As of now, it is purely cosmetic.
Entity createToolbar()
{
	// Create the associated entity.
	Entity toolbar_entity = Entity();

	// Create the associated component.
	Toolbar& toolbar_component = registry.toolbars.emplace(toolbar_entity);

	// Create a component to simplify movement.
	MoveWithCamera& mwc = registry.moveWithCameras.emplace(toolbar_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(toolbar_entity);
	motion_component.position = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX * 1.05);
	motion_component.scale = vec2(960, 120);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		toolbar_entity,
		{
			TEXTURE_ASSET_ID::TOOLBAR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return toolbar_entity;
}

Entity createGameOver() {
	Entity entity = Entity();
	
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 };
	motion.scale = vec2({ WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX });


	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GAMEOVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);
	return entity;
}

// Kung: Create the pause button that will eventually pause the game.
// As of now, it is purely cosmetic.
Entity createPause()
{
	// Create the associated entity.
	Entity pause_entity = Entity();

	// Create the associated component.
	Pause& pause_component = registry.pauses.emplace(pause_entity);

	// Create a component to simplify movement.
	MoveWithCamera& mwc = registry.moveWithCameras.emplace(pause_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(pause_entity);
	motion_component.position = vec2(-150, -50);
	motion_component.scale = vec2(120, 120);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		pause_entity,
		{
			TEXTURE_ASSET_ID::PAUSE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return pause_entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 position) {
	Entity entity = Entity();

	State& state = registry.states.emplace(entity);
	state.state = STATE::IDLE;

	Player& player = registry.players.emplace(entity);
	player.health = PLAYER_HEALTH;
	
	MoveWithCamera& mwc = registry.moveWithCameras.emplace(entity);

	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ PLAYER_HEIGHT, PLAYER_HEIGHT });

	Attack& attack = registry.attacks.emplace(entity);
	attack.range = 60;

	registry.statuses.emplace(entity);

	//create detection box
	// createDetectionLine(entity, vec2{position.x+30, position.y-30}, vec2{attack.range, 2});                       //upper -----
	// createDetectionLine(entity, vec2{position.x+attack.range, position.y}, vec2{2, PLAYER_BB_HEIGHT});           //          |
	// createDetectionLine(entity, vec2{position.x+30, position.y-30+PLAYER_BB_HEIGHT}, vec2{attack.range, 2});     //lower -----

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER_IDLE,
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);

	//grey box
	// vec3& cv = registry.colors.emplace(entity);
	// cv.r = 0.5;
	// cv.g = 0.5;
	// cv.b = 0.5;
	

	// registry.renderRequests.insert(
	// 	entity,
	// 	{
	// 		TEXTURE_ASSET_ID::TEXTURE_COUNT,
	// 		EFFECT_ASSET_ID::COLOURED,
	// 		GEOMETRY_BUFFER_ID::SPRITE
	// 	}
	// );
	return entity;
}

/**
* Create a slash animation.
* 
* @param renderer The renderer.
* @param position The position of the slash.
* @param scale The scale of the slash.
* @return The slash entity.
*/
Entity createEffect(RenderSystem* renderer, vec2 position, vec2 scale) {
	Entity entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);

	AnimationSystem::update_animation(entity, SLASH_FRAME_DELAY, SLASH_ANIMATION, sizeof(SLASH_ANIMATION) / sizeof(SLASH_ANIMATION[0]), false, false, true);

	return entity;
}

// Kung: Create the seed that will be planted whenever there is farmland.
Entity createSeed(vec2 pos)
{
	// Create the associated entity.
	Entity seed_entity = Entity();

	// Create the associated component.
	Seed& seed_component = registry.seeds.emplace(seed_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(seed_entity);
	motion_component.position = pos;
	motion_component.scale = vec2(50, 50);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		seed_entity,
		{
			TEXTURE_ASSET_ID::SEED_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return seed_entity;
}

// Kung: Create the seed that appears within the toolbar.
Entity createSeedInventory(int level)
{
	// Create the associated entity.
	Entity seed_entity = Entity();

	// Create the associated component.
	Seed& seed_component = registry.seeds.emplace(seed_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(seed_entity);
	motion_component.position = vec2(0, 0);
	motion_component.scale = vec2(50, 50);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		seed_entity,
		{
			TEXTURE_ASSET_ID::SEED_1,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return seed_entity;
}

// create a camera
Entity createCamera(RenderSystem* renderer,vec2 position) {
    // Create camera entity
    Entity camera = Entity();

    // Create camera component
    Camera& camera_component = registry.cameras.emplace(camera);
    camera_component.position = position;

    
    return camera;
}