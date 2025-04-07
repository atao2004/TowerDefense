#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include "animation_system.hpp"
#include "../ext/json.hpp"
#include "particle_system.hpp"
using json = nlohmann::json;

// Entity createGridLine(vec2 start_pos, vec2 end_pos)
// {
// 	Entity entity = Entity();
// 	GridLine &gl = registry.gridLines.emplace(entity);
// 	gl.start_pos = start_pos;
// 	gl.end_pos = end_pos;

// 	registry.renderRequests.insert(
// 		entity,
// 		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
// 		 EFFECT_ASSET_ID::EGG,
// 		 GEOMETRY_BUFFER_ID::DEBUG_LINE});

// 	vec3 &cv = registry.colors.emplace(entity);
// 	cv = GRID_COLOR;

// 	return entity;
// }

Entity createPausePanel(RenderSystem* renderer, vec2 position) {
	Entity entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ 800, 700 });
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PAUSE_PANEL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		false
	);
	registry.cgs.emplace(entity);
	return entity;
}

Entity createButton(BUTTON_ID type, vec2 position, vec2 toDeduct) {
	Entity entity = Entity();
	CustomButton &button = registry.buttons.emplace(entity);
	button.type = type;
	// std::cout<<button.position.x<<" "<<button.position.y<<std::endl;
	button.position = toDeduct;
	
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = vec2({BUTTON_SPLASH_WIDTH, BUTTON_SPLASH_HEIGHT});
	registry.renderRequests.insert(
		entity,
		{(TEXTURE_ASSET_ID)((int)TEXTURE_ASSET_ID::START_BUTTON + (int)type),
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});
	registry.cgs.emplace(entity);
	return entity;
}

Entity createScreen(TEXTURE_ASSET_ID background)
{
	Entity entity = Entity();
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2};
	motion.scale = vec2({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});
	registry.renderRequests.insert(
		entity,
		{background,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});
	registry.cgs.emplace(entity);
	return entity;
}

// createZombieSpawn
Entity createZombieSpawn(RenderSystem *renderer, vec2 position)
{
	Entity entity = Entity();

	registry.zombieSpawns.emplace(entity);

	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = vec2({ENEMY_WIDTH, ENEMY_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ZOMBIE_SPAWN_1,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE},
		false);

	AnimationSystem::update_animation(entity, ZOMBIE_SPAWN_DURATION, ZOMBIE_SPAWN_ANIMATION, ZOMBIE_SPAWN_SIZE, false, false, true);

	return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, int health, int damage, int speed, int anim_duration, const TEXTURE_ASSET_ID *anim_textures, int anim_size)
{
	auto entity = Entity();

	Zombie &zombie = registry.zombies.emplace(entity);

	Attack &attack = registry.attacks.emplace(entity);
	attack.range = 30.0f;
	attack.damage = damage;

	Enemy &enemy = registry.enemies.emplace(entity);
	enemy.health = health;
	enemy.speed = speed;

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = vec2({ENEMY_WIDTH, ENEMY_HEIGHT});

	VisualScale &vscale = registry.visualScales.emplace(entity);
	vscale.scale = {5.f, 5.f}; // Scale visuals 3.1x

	registry.renderRequests.insert(
		entity,
		{anim_textures[0],
		 EFFECT_ASSET_ID::ZOMBIE,
		 GEOMETRY_BUFFER_ID::SPRITE},
		false);

	AnimationSystem::update_animation(entity, anim_duration, anim_textures, anim_size, true, false, false);

	return entity;
}

Entity createOrc(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, ORC_HEALTH, ORC_DAMAGE, ORC_SPEED, ORC_ANIMATION_DURATION, ORC_ANIMATION, ORC_ANIMATION_SIZE);
}

Entity createOrcElite(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, ORC_ELITE_HEALTH, ORC_ELITE_DAMAGE, ORC_ELITE_SPEED, ORC_ELITE_ANIMATION_DURATION, ORC_ELITE_ANIMATION, ORC_ELITE_ANIMATION_SIZE);
}

Entity createSkeleton(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, SKELETON_HEALTH, SKELETON_DAMAGE, SKELETON_SPEED, SKELETON_ANIMATION_DURATION, SKELETON_ANIMATION, SKELETON_ANIMATION_SIZE);
}

Entity createWerebear(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, WEREBEAR_HEALTH, WEREBEAR_DAMAGE, WEREBEAR_SPEED, WEREBEAR_ANIMATION_DURATION, WEREBEAR_ANIMATION, WEREBEAR_ANIMATION_SIZE);
}

Entity createWerewolf(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, WEREWOLF_HEALTH, WEREWOLF_DAMAGE, WEREWOLF_SPEED, WEREWOLF_ANIMATION_DURATION, WEREWOLF_ANIMATION, WEREWOLF_ANIMATION_SIZE);
}

Entity createSlime(RenderSystem *renderer, vec2 position)
{
	return createEnemy(renderer, position, SLIME_HEALTH, SLIME_DAMAGE, SLIME_SPEED, SLIME_ANIMATION_DURATION, SLIME_ANIMATION, SLIME_ANIMATION_SIZE);
}

Entity createOrcRider(RenderSystem *renderer, vec2 position)
{
    Entity entity = Entity();

    Motion &motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.scale = vec2(60.0f, 60.0f);

    // Add zombie component 
    Zombie &zombie = registry.zombies.emplace(entity);

    // Add enemy component
    Enemy &enemy = registry.enemies.emplace(entity);
    enemy.health = 80; 
    enemy.speed = 100;  // Base speed same as walk speed
    
    Attack &attack = registry.attacks.emplace(entity);
    attack.range = 60.0f;         // Melee range for charge attack
    attack.damage = 25;           // Same as orcrider.damage
    
    VisualScale &vscale = registry.visualScales.emplace(entity);
    vscale.scale = {4.f, 4.f}; // Scale visuals 4x
    
    OrcRider &orcrider = registry.orcRiders.emplace(entity);
    orcrider.damage = 25; // Damage when charging into player

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ORC_RIDER_IDLE1,
		 EFFECT_ASSET_ID::ZOMBIE,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Start with idle animation using the pre-defined constants
	AnimationSystem::update_animation(
		entity,
		ORCRIDER_IDLE_ANIMATION_DURATION,
		ORCRIDER_IDLE_ANIMATION,
		ORCRIDER_IDLE_ANIMATION_SIZE,
		true,  // loop
		false, // not locked
		false  // don't destroy
	);

	return entity;
}

Entity createPlant(RenderSystem* renderer, vec2 position, PLANT_ID id)
{
	Entity entity = Entity();

	// Basic tower stats
	Tower &tower = registry.towers.emplace(entity);
	tower.health = PLANT_STATS_MAP.at(id).health;
	tower.damage = PLANT_STATS_MAP.at(id).damage;
	tower.range = PLANT_STATS_MAP.at(id).range; // Detection range in pixels
	tower.timer_ms = PLANT_STATS_MAP.at(id).cooldown;
	tower.state = false;
	tower.type = PLANT_STATS_MAP.at(id).type;

	// Motion component for position and rotation
	Motion &motion = registry.motions.emplace(entity);
	motion.position = {position.x + TOWER_BB_WIDTH / 2, position.y + TOWER_BB_HEIGHT / 2};
	motion.angle = 0.f;
	motion.velocity = {0, 0};								// Towers don't move
	motion.scale = vec2({TOWER_BB_WIDTH, TOWER_BB_HEIGHT}); // Using constants from common.hpp

	Dimension &dimension = registry.dimensions.emplace(entity);
	dimension.width = TOWER_BB_WIDTH;
	dimension.height = TOWER_BB_HEIGHT;

	VisualScale& vscale = registry.visualScales.emplace(entity);
	vscale.scale = { 1.5f, 1.5f };

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Plant animation id
	PlantAnimation& plant_animation = registry.plantAnimations.emplace(entity);
	plant_animation.id = id;

	// Add render request for tower
	registry.renderRequests.insert(
		entity,
		{PLANT_ANIMATION_MAP.at(id).idle.textures[0],
		 EFFECT_ASSET_ID::ZOMBIE,
		 GEOMETRY_BUFFER_ID::SPRITE});

	AnimationSystem::update_animation(entity, PLANT_ANIMATION_MAP.at(id).idle.duration, PLANT_ANIMATION_MAP.at(id).idle.textures, PLANT_ANIMATION_MAP.at(id).idle.size, true, false, false);

	return entity;
}

// Kung: Create the grass texture that will be used as part of the texture map.
Entity createMapTile(Entity maptile_entity, vec2 position)
{
	// Create the associated component.
	MapTile &maptile_component = registry.mapTiles.emplace(maptile_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(maptile_entity);
	motion_component.position = position;
	motion_component.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		maptile_entity,
		{DECORATION_LIST[1],
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return maptile_entity;
}

// Helper function to make it easier to create tutorial tiles.
Entity createMapTile(vec2 position)
{
	// Create the associated entity.
	Entity maptile_entity = Entity();

	// Continue in the main function.
	return createMapTile(maptile_entity, position);
}

// Create decorations to overlay on the map.
Entity createMapTileDecoration(Entity decoration_entity, int i, vec2 position)
{
	// Create the associated component.
	MapTile &maptile_component = registry.mapTiles.emplace(decoration_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(decoration_entity);
	motion_component.position = position;
	motion_component.scale = DECORATION_SIZE_LIST[i];
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		decoration_entity,
		{DECORATION_LIST[i],
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return decoration_entity;
}

// Helper function to make it easier to create tutorial tiles.
Entity createMapTileDecoration(int i, vec2 position)
{
	// Create the associated entity.
	Entity decoration_entity = Entity();

	// Continue in the main function.
	return createMapTileDecoration(decoration_entity, i, position);
}

Entity createTutorialTile(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_tile_entity = Entity();

	// Create the associated component.
	TutorialTile &tutorial_tile_component = registry.tutorialTiles.emplace(tutorial_tile_entity);

	// Create the associated maptile component.
	createMapTile(tutorial_tile_entity, position);

	return tutorial_tile_entity;
}

Entity createTutorialTileDecoration(int i, vec2 position)
{
	// Create the associated entity.
	Entity tutorial_tile_entity = Entity();

	// Create the associated component.
	TutorialTile &tutorial_tile_component = registry.tutorialTiles.emplace(tutorial_tile_entity);

	// Create the associated maptile component.
	createMapTileDecoration(tutorial_tile_entity, i, position);

	return tutorial_tile_entity;
}

// Kung: Create the tiles that border the window and represent areas in which the player cannot walk on.
// Technically, the tiles represent dirt, but will look different from the farmland that will be implemented later.
Entity createScorchedEarth(vec2 position)
{
	// Create the associated entity.
	Entity scorched_earth_entity = Entity();

	// Create the associated component.
	ScorchedEarth &scorched_earth_component = registry.scorchedEarths.emplace(scorched_earth_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(scorched_earth_entity);
	motion_component.position = position;
	motion_component.scale = vec2(GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		scorched_earth_entity,
		{TEXTURE_ASSET_ID::SCORCHED_EARTH,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return scorched_earth_entity;
}

// Kung: Remove all surfaces before adding in new ones when resetting the game.
// This includes grass and scorched earth, and eventually farmland as well.
void removeSurfaces()
{
	// remove all mapTiles
	for (Entity &maptile_entity : registry.mapTiles.entities)
	{
		registry.remove_all_components_of(maptile_entity);
	}
	// remove all scorched earth
	for (Entity &scorched_earth_entity : registry.scorchedEarths.entities)
	{
		registry.remove_all_components_of(scorched_earth_entity);
	}
	// print confirmation
	std::cout << "surfaces reset" << std::endl;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialMove(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_MOVE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[8] = {
		TEXTURE_ASSET_ID::TUTORIAL_MOVE,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE_W,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE_A,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE_S,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE,
		TEXTURE_ASSET_ID::TUTORIAL_MOVE_D,
	};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 8;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialAttack(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_ATTACK,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[2] = {
		TEXTURE_ASSET_ID::TUTORIAL_ATTACK,
		TEXTURE_ASSET_ID::TUTORIAL_ATTACK_ANIMATED};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 2;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialPlant(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_PLANT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[2] = {
		TEXTURE_ASSET_ID::TUTORIAL_PLANT,
		TEXTURE_ASSET_ID::TUTORIAL_PLANT_ANIMATED};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 2;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialDash(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_DASH,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[3] = {
		TEXTURE_ASSET_ID::TUTORIAL_DASH,
		TEXTURE_ASSET_ID::TUTORIAL_DASH_ANIMATED,
		TEXTURE_ASSET_ID::TUTORIAL_DASH_ANIMATED_2};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 3;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialChangeSeed(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[9] = {
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_1,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_2,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_3,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_4,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_5,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_6,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_7,
		TEXTURE_ASSET_ID::TUTORIAL_CHANGE_SEED_8};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 9;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create a sign that will only appear once it appears in a tutorial.
Entity createTutorialRestart(vec2 position)
{
	// Create the associated entity.
	Entity tutorial_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(tutorial_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(tutorial_entity);
	motion_component.position = position;
	motion_component.scale = vec2(465, 345);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		tutorial_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_RESTART,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Create the animation
	static TEXTURE_ASSET_ID asset_id_array[2] = {
		TEXTURE_ASSET_ID::TUTORIAL_RESTART,
		TEXTURE_ASSET_ID::TUTORIAL_RESTART_ANIMATED};

	Animation &animation_component = registry.animations.emplace(tutorial_entity);
	animation_component.transition_ms = 1000;
	animation_component.pose_count = 2;
	animation_component.loop = true;
	animation_component.lock = true;
	animation_component.textures = asset_id_array;

	return tutorial_entity;
}

// Create an arrow to go along with the tutorial map.
Entity createTutorialArrow(vec2 position)
{
	// Create the associated entity.
	Entity arrow_entity = Entity();

	// Create the associated component.
	TutorialSign &tutorial_component = registry.tutorialSigns.emplace(arrow_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(arrow_entity);
	motion_component.position = position;
	motion_component.scale = vec2(80, 150);
	motion_component.velocity = vec2(0, 0);

	// Render the sign.
	registry.renderRequests.insert(
		arrow_entity,
		{TEXTURE_ASSET_ID::TUTORIAL_ARROW,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return arrow_entity;
}

void parseMap(bool tutorial)
{
	json jsonFile;
	if (tutorial)
	{
		std::ifstream file(PROJECT_SOURCE_DIR + std::string("data/map/tutorialMap.json"));
		file >> jsonFile;
	}
	else
	{
		std::ifstream file(PROJECT_SOURCE_DIR + std::string("data/map/myMap.json"));
		file >> jsonFile;
	}

	int numCol = jsonFile["width"];
	int numRow = jsonFile["height"];
	std::vector<int> map_layer = jsonFile["layers"][0]["data"];
	std::vector<int> decoration_layer = jsonFile["layers"][1]["data"];

	//// create background
	// for (int i = 0; i < numRow; i++)
	//{ // iterating row-by-row
	//	for (int j = 0; j < numCol; j++)
	//	{
	//		if (map_layer[i * numCol + j] == 1)
	//		{
	//			if (tutorial)
	//			{
	//				createTutorialTile({j * GRID_CELL_WIDTH_PX, i * GRID_CELL_HEIGHT_PX});
	//			}
	//			else
	//			{
	//				createMapTile({j * GRID_CELL_WIDTH_PX, i * GRID_CELL_HEIGHT_PX});
	//			}
	//		}
	//	}
	// }

	// add decorations
	for (int i = 0; i < numRow; i++)
	{ // iterating row-by-row
		for (int j = 0; j < numCol; j++)
		{
			if (decoration_layer[i * numCol + j] != 0 && decoration_layer[i * numCol + j] != 1)
			{
				if (tutorial)
				{
					createTutorialTileDecoration(decoration_layer[i * numCol + j], {j * GRID_CELL_WIDTH_PX, i * GRID_CELL_HEIGHT_PX});
				}
				else
				{
					createMapTileDecoration(decoration_layer[i * numCol + j], {j * GRID_CELL_WIDTH_PX, i * GRID_CELL_HEIGHT_PX});
				}
			}
		}
	}
}

// Kung: Create the toolbar that in the future will store seeds, harvests, and other associated items.
Entity createToolbar(vec2 position)
{
	// Create the associated entity.
	Entity toolbar_entity = Entity();
	Entity projectile = Entity();

    Motion &proj_motion = registry.motions.emplace(projectile);
	proj_motion.position.y = position.y;
    proj_motion.position.x = position.x - 4*TOOLBAR_WIDTH / 8 + TOOLBAR_HEIGHT / 2;
    proj_motion.scale = vec2(60,60);

     proj_motion.velocity = {0,0};

    registry.renderRequests.insert(
        projectile,
        {TEXTURE_ASSET_ID::PROJECTILE,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE},
		false);

	// Create the associated component.
	Toolbar &toolbar_component = registry.toolbars.emplace(toolbar_entity);
	Toolbar &projectile_component = registry.toolbars.emplace(projectile);
	

	// Create a component to simplify movement.
	MoveWithCamera &mwc = registry.moveWithCameras.emplace(toolbar_entity);
	MoveWithCamera &mwcc = registry.moveWithCameras.emplace(projectile);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(toolbar_entity);
	motion_component.position = position;
	motion_component.scale = vec2(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		toolbar_entity,
		{TEXTURE_ASSET_ID::TOOLBAR,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return toolbar_entity;
}

Entity createGameOver()
{
	registry.animations.clear();
	registry.deathAnimations.clear();
	registry.renderRequests.clear();
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = {WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2};
	motion.scale = vec2({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});
	
	return entity;
}

// Kung: Create the pause button that will eventually pause the game.
// As of now, it is purely cosmetic.
Entity createPause(vec2 position)
{
	// Create the associated entity.
	Entity pause_entity = Entity();

	// Create a component to simplify movement.
	MoveWithCamera &mwc = registry.moveWithCameras.emplace(pause_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(pause_entity);
	motion_component.position = position;
	motion_component.scale = vec2(60, 60);
	motion_component.velocity = vec2(0, 0);

	CustomButton& button = registry.buttons.emplace(pause_entity);
	button.type = BUTTON_ID::PAUSE;
	button.position = vec2(30, 30);

	// Render the object.
	registry.renderRequests.insert(
		pause_entity,
		{TEXTURE_ASSET_ID::PAUSE_BUTTON,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return pause_entity;
}

Entity createPlayer(RenderSystem *renderer, vec2 position, int seed_type)
{
	Entity entity = Entity();

	State &state = registry.states.emplace(entity);
	state.state = STATE::IDLE;

	Player &player = registry.players.emplace(entity);
	player.health = PLAYER_HEALTH;
	player.health_max = PLAYER_HEALTH;

	Inventory &inventory = registry.inventorys.emplace(entity);
	registry.inventorys.components[0].seedCount[seed_type] = 5; // 5 starter seeds
	for(int i = 0; i < NUM_SEED_TYPES; i++)
	{
		registry.inventorys.components[0].seedCount[i] = 4; // one each of the 8 seed types
		registry.inventorys.components[0].seedAtToolbar[i] = i;
	}

	MoveWithCamera &mwc = registry.moveWithCameras.emplace(entity);
	Motion &motion = registry.motions.emplace(entity);

	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = vec2({PLAYER_HEIGHT, PLAYER_HEIGHT});

	VisualScale &vscale = registry.visualScales.emplace(entity);
	vscale.scale = {5.f, 5.f}; // Scale visuals 3.1x

	Attack &attack = registry.attacks.emplace(entity);
	attack.range = 60;
	attack.damage = PLAYER_DAMAGE;

	registry.statuses.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PLAYER_IDLE1,
		 EFFECT_ASSET_ID::PLAYER,
		 GEOMETRY_BUFFER_ID::SPRITE},
		false);

	AnimationSystem::update_animation(entity, PLAYER_IDLE_DURATION, PLAYER_IDLE_ANIMATION, PLAYER_IDLE_SIZE, true, false, false);

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
Entity createEffect(RenderSystem *renderer, vec2 position, vec2 scale)
{
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PLAYER_ATTACK_SLASH_1,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE},
		false);

	AnimationSystem::update_animation(entity, SLASH_DURATION, SLASH_ANIMATION, SLASH_SIZE, false, false, true);

	return entity;
}

// Kung: Create the seed that will be planted whenever there is farmland.
Entity createSeed(vec2 pos, int type)
{
	// Create the associated entity.
	Entity seed_entity = Entity();

	// Create the associated component.
	Seed &seed_component = registry.seeds.emplace(seed_entity);
	seed_component.type = type;
	seed_component.timer = SEED_MATURE_SPEED * 1000;

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(seed_entity);
	motion_component.position = pos;
	motion_component.scale = vec2(50, 50);
	motion_component.velocity = vec2(0, 0);

	// Render the object.
	registry.renderRequests.insert(
		seed_entity,
		{SEED_MAP.at(type).texture,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	ParticleSystem::createSeedGrowthEffect(pos, motion_component.scale);

	return seed_entity;
}

// Kung: Create the seed that appears within the toolbar.
Entity createSeedInventory(vec2 pos, vec2 velocity, int type, int toolbar_pos)
{
	// Create the associated entity.
	Entity seed_entity = Entity();

	// Create the associated component.
	Seed &seed_component = registry.seeds.emplace(seed_entity);
	seed_component.type = type;
	seed_component.timer = 5000;

	// Create a component to simplify movement.
	MoveWithCamera &mwc = registry.moveWithCameras.emplace(seed_entity);

	// Create the relevant motion component.
	Motion &motion_component = registry.motions.emplace(seed_entity);
	motion_component.position = pos;
	motion_component.scale = vec2(50, 50);
	motion_component.velocity = velocity;

	// Render the object.
	registry.renderRequests.insert(
		seed_entity,
		{
			SEED_MAP.at(type).texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return seed_entity;
}

// create a camera
Entity createCamera(RenderSystem *renderer, vec2 position)
{
	// Create camera entity
	Entity camera = Entity();

	// Create camera component
	Camera &camera_component = registry.cameras.emplace(camera);
	camera_component.position = position;

	return camera;
}

Entity createSkeletonArcher(RenderSystem *renderer, vec2 position)
{
	// Create base entity
	Entity entity = Entity();

	// Add skeleton specific component with improved properties
	Skeleton &skeleton = registry.skeletons.emplace(entity);
	skeleton.attack_range = 400.0f;							// Longer range than zombies
	skeleton.stop_distance = 300.0f;						// Stops moving at this distance
	skeleton.attack_cooldown_ms = SKELETON_ATTACK_DURATION; // Attack cooldown

	Enemy &enemy = registry.enemies.emplace(entity);
	enemy.health = SKELETON_ARCHER_HEALTH;

	// Add attack component separate from zombies
	Attack &attack = registry.attacks.emplace(entity);
	attack.range = skeleton.attack_range;	// Match the attack range
	attack.damage = SKELETON_ARCHER_DAMAGE; // Set the damage value

	// Add motion component
	Motion &motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.velocity = {0, 0};
	motion.scale = {ENEMY_WIDTH, ENEMY_HEIGHT};

	VisualScale &vscale = registry.visualScales.emplace(entity);
	vscale.scale = {5.f, 5.f}; // Scale visuals 3.1x

	// Add render request - temporarily use zombie texture
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::SKELETON_IDLE1, // Replace with SKELETON texture
		 EFFECT_ASSET_ID::ZOMBIE,		   // Consider using a unique shader for skeletons
		 GEOMETRY_BUFFER_ID::SPRITE});

	// Add animation using zombie textures until skeleton textures are added
	// Add idle animation
	AnimationSystem::update_animation(
		entity,
		800,					 // Duration in ms (slower for idle)
		SKELETON_IDLE_ANIMATION, // Idle animation texture array
		6,						 // Number of frames (6)
		true,					 // Loop animation
		false,					 // Not locked (can be replaced)
		false					 // Don't destroy when done
	);

	return entity;
}

// Create an arrow projectile
Entity createArrow(vec2 position, vec2 direction, Entity source)
{
	// Create entity
	Entity entity = Entity();

	// Add arrow component
	Arrow &arrow = registry.arrows.emplace(entity);
	arrow.source = source;
	arrow.direction = normalize(direction); // Ensure direction is normalized
	arrow.damage = SKELETON_ARCHER_DAMAGE;

	// Add motion component
	Motion &motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.velocity = arrow.direction * arrow.speed;
	// Set proper angle for the arrow based on direction
	motion.angle = atan2(direction.y, direction.x) * 180.f / M_PI;
	motion.scale = {10.f, 10.f}; // Arrow size

	VisualScale &vscale = registry.visualScales.emplace(entity);
	vscale.scale = {10.f, 10.f}; // Scale visuals 2.5x

	// Add render request - use projectile texture or a specific arrow texture
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::ARROW,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createText(std::string text, vec2 pos, float size, vec3 color) {
	Entity text_entity = Entity();

	Text &text_component = registry.texts.emplace(text_entity);
	text_component.text = text;
	text_component.pos = pos;
	text_component.size = size;
	text_component.color = color;
	
	return text_entity;
}

Entity createChicken(RenderSystem *renderer)
{
	auto entity = Entity();

	Projectile &projectile = registry.projectiles.emplace(entity);
	projectile.damage = CHICKEN_DAMAGE;
	projectile.invincible = true;

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::CHICKEN);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {CHICKEN_SPEED, 0};
	motion.position = vec2(0, WINDOW_HEIGHT_PX / 2);
	motion.scale = mesh.original_size * CHICKEN_SIZE;
	motion.scale.x *= -1;
	motion.scale.y *= -1;

	if (registry.players.size() > 0)
	{
		Entity &player_entity = registry.players.entities[0];
		Motion &player_motion = registry.motions.get(player_entity);
		motion.position.y = player_motion.position.y;
	}

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::CHICKEN,
		 GEOMETRY_BUFFER_ID::CHICKEN});

	return entity;
}

Entity createCharacter(RenderSystem *renderer, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture)
{
	Entity entity = Entity();
	Motion &motion = registry.motions.emplace(entity);

	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{texture,
		 EFFECT_ASSET_ID::PLAYER,
		 GEOMETRY_BUFFER_ID::SPRITE},
		false);
	registry.cgs.emplace(entity);
	return entity;
}
