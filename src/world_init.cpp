#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

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

// Entity createDetectionLine(Entity entity, vec2 start_pos, vec2 end_pos) {
// 	GridLine& gl = registry.gridLines.emplace_with_duplicates(entity);
// 	gl.start_pos = start_pos;
// 	gl.end_pos = end_pos;

// 	registry.renderRequests.insert(
// 		entity,
// 		{
// 			TEXTURE_ASSET_ID::TEXTURE_COUNT,
// 			EFFECT_ASSET_ID::EGG,
// 			GEOMETRY_BUFFER_ID::DEBUG_LINE
// 		},
// 		false
// 	);
// 	//only need to create 1 color so all the lines can use the same color
// 	if (!registry.colors.has(entity)) {
// 		vec3& cv = registry.colors.emplace(entity);
// 		cv.r = 1;
// 		cv.g = 0;
// 		cv.b = 0;
// 	}

// 	return entity;
// }

Entity createZombie(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Zombie& zombie = registry.zombies.emplace(entity);
	zombie.health = ZOMBIE_HEALTH;

	Attack& attack = registry.attacks.emplace(entity);
	attack.range = 30.0f;         

	Animation& animation = registry.animations.emplace(entity);
	animation.transition_ms = ZOMBIE_MOVE_FRAME_DELAY;
	animation.pose_count = sizeof(ZOMBIE_ANIMATION) / sizeof(ZOMBIE_ANIMATION[0]);
	animation.textures = ZOMBIE_ANIMATION;

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

	// Enemy Count update:
    std::cout << "Enemy count: " << registry.zombies.size() << " zombies" << std::endl;

	return entity;
}

Entity createTower(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto& t = registry.towers.emplace(entity);
	t.range = (float)WINDOW_WIDTH_PX / (float)GRID_CELL_WIDTH_PX;
	t.timer_ms = 1000;	// arbitrary for now

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;	// A1-TD: CK: rotate to the left 180 degrees to fix orientation
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	std::cout << "INFO: tower position: " << position.x << ", " << position.y << std::endl;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -TOWER_BB_WIDTH, TOWER_BB_HEIGHT });

	// create an (empty) Tower component to be able to refer to all towers
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

Entity createGrass(vec2 position)
{
	Entity grass_entity = Entity();

	Grass& grass_component = registry.grasses.emplace(grass_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(grass_entity);
	motion_component.position = position;
	motion_component.scale = vec2(GRASS_DIMENSION_PX, GRASS_DIMENSION_PX);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		grass_entity,
		{
			TEXTURE_ASSET_ID::GRASS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return grass_entity;
}

Entity createScorchedEarth(vec2 position)
{
	Entity scorched_earth_entity = Entity();

	ScorchedEarth& scorched_earth_component = registry.scorchedEarths.emplace(scorched_earth_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(scorched_earth_entity);
	motion_component.position = position;
	motion_component.scale = vec2(DIRT_DIMENSION_PX, DIRT_DIMENSION_PX);
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

void removeSurfaces()
{
	// remove all grasses
	for (Entity& grass_entity : registry.grasses.entities) {
		registry.remove_all_components_of(grass_entity);
	}
	for (Entity& scorched_earth_entity : registry.scorchedEarths.entities) {
		registry.remove_all_components_of(scorched_earth_entity);
	}
	std::cout << "surfaces reset" << std::endl;
}

Entity createToolbar()
{
	Entity toolbar_entity = Entity();

	Toolbar& toolbar_component = registry.toolbars.emplace(toolbar_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(toolbar_entity);
	motion_component.position = vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 50);
	motion_component.scale = vec2(600, 75);
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

Entity createPause()
{
	Entity pause_entity = Entity();

	Pause& pause_component = registry.pauses.emplace(pause_entity);

	// Create the relevant motion component.
	Motion& motion_component = registry.motions.emplace(pause_entity);
	motion_component.position = vec2(50, 50);
	motion_component.scale = vec2(50, 50);
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
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);

	Animation& animation = registry.animations.emplace(entity);

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