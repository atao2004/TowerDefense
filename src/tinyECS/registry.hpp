#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	ComponentContainer<Attack> attacks;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<Dimension> dimensions;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<vec3> colors;
	ComponentContainer<Tower> towers;
	ComponentContainer<GridLine> gridLines;
	ComponentContainer<Grass> grasses;
	ComponentContainer<Farmland> farmlands;
	ComponentContainer<ScorchedEarth> scorchedEarths;
	ComponentContainer<Toolbar> toolbars;
	ComponentContainer<Pause> pauses;
	ComponentContainer<MoveWithCamera> moveWithCameras;

	ComponentContainer<Zombie> zombies;
	ComponentContainer<Player> players;
	ComponentContainer<StatusComponent> statuses;
	ComponentContainer<State> states;
	ComponentContainer<Animation> animations;
	ComponentContainer<Seed> seeds;

	ComponentContainer<Death> deaths;
	ComponentContainer<Cooldown> cooldowns;
	ComponentContainer<DeathAnimation> deathAnimations;
	ComponentContainer<HitEffect> hitEffects;

	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Camera> cameras;

	// constructor that adds all containers for looping over them
	ECSRegistry() {
		registry_list.push_back(&attacks);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&dimensions);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&colors);
		registry_list.push_back(&towers);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&zombies);
		registry_list.push_back(&players);
		registry_list.push_back(&statuses);
		registry_list.push_back(&states);
		registry_list.push_back(&animations);
		registry_list.push_back(&deaths);
		registry_list.push_back(&cooldowns);
		registry_list.push_back(&deathAnimations);
		registry_list.push_back(&hitEffects);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&cameras);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;