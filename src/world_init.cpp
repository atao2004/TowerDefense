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
	cv.r = 0;
	cv.g = 0;
	cv.b = 1;

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

	registry.zombies.emplace(entity);

    Creature& creature = registry.creatures.emplace(entity);
    creature.health = ZOMBIE_HEALTH;

	Attack& attack = registry.attacks.emplace(entity);
    attack.range = 30.0f;         
    attack.damage = 10.0f;        
    attack.cooldown_ms = 1000.0f;

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
			TEXTURE_ASSET_ID::ZOMBIE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

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

Entity createPlayer(RenderSystem* renderer, vec2 position) {
	Entity entity = Entity();

	registry.players.emplace(entity);
	Creature& creature = registry.creatures.emplace(entity);
	creature.health = PLAYER_HEALTH;
	
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ PLAYER_WIDTH, PLAYER_HEIGHT });

	Attack& attack = registry.attacks.emplace(entity);
	attack.damage = 10;
	attack.range = 60;

	registry.statuses.emplace(entity);

	//create detection box
	// createDetectionLine(entity, vec2{position.x+30, position.y-30}, vec2{attack.range, 2});                       //upper -----
	// createDetectionLine(entity, vec2{position.x+attack.range, position.y}, vec2{2, INVADER_BB_HEIGHT});           //          |
	// createDetectionLine(entity, vec2{position.x+30, position.y-30+INVADER_BB_HEIGHT}, vec2{attack.range, 2});     //lower -----

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER_IDLE,
			EFFECT_ASSET_ID::TEXTURED,
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