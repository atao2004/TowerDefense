#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	

public:
// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;
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
	ComponentContainer<MapTile> mapTiles;
	ComponentContainer<ScorchedEarth> scorchedEarths;
	ComponentContainer<TutorialTile> tutorialTiles;
	ComponentContainer<TutorialSign> tutorialSigns;
	ComponentContainer<Inventory> inventorys;
	
	ComponentContainer<Toolbar> toolbars;
	ComponentContainer<Pause> pauses;
	ComponentContainer<MoveWithCamera> moveWithCameras;

	ComponentContainer<Zombie> zombies;
	ComponentContainer<ZombieSpawn> zombieSpawns;
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
	ComponentContainer<Skeleton> skeletons;
	ComponentContainer<Arrow> arrows;
	ComponentContainer<VisualScale> visualScales;

	ComponentContainer<Enemy> enemies;
	ComponentContainer<Button> buttons;
	ComponentContainer<Particle> particles;
	ComponentContainer<ParticleGenerator> particleGenerators;

	// constructor that adds all containers for looping over them
	ECSRegistry() {
		registry_list.push_back(&screenStates); //0
		registry_list.push_back(&attacks);//1
		registry_list.push_back(&motions);//2
		registry_list.push_back(&collisions);//3
		registry_list.push_back(&meshPtrs);//4
		registry_list.push_back(&dimensions);//5
		registry_list.push_back(&renderRequests);//6
		registry_list.push_back(&colors);//7
		registry_list.push_back(&towers);//8
		registry_list.push_back(&gridLines);//9
		registry_list.push_back(&zombies);//10
		registry_list.push_back(&zombieSpawns);//11
		registry_list.push_back(&players);//12
		registry_list.push_back(&statuses);//13
		registry_list.push_back(&states);//14
		registry_list.push_back(&animations);//15
		registry_list.push_back(&deaths);//16
		registry_list.push_back(&cooldowns);//17
		registry_list.push_back(&deathAnimations);//18
		registry_list.push_back(&hitEffects);//19
		registry_list.push_back(&projectiles);//20
		registry_list.push_back(&cameras);//21
		registry_list.push_back(&skeletons);//22
		registry_list.push_back(&arrows);//23
		registry_list.push_back(&visualScales);//24
		registry_list.push_back(&enemies);//25
		registry_list.push_back(&inventorys);//26
		registry_list.push_back(&seeds);//27
		registry_list.push_back(&moveWithCameras);//28
		registry_list.push_back(&particles);//29
		registry_list.push_back(&particleGenerators);//30
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list) {
			if (!dynamic_cast<ComponentContainer<ScreenState>*>(reg))    //do not remove screenstate
				reg->clear();
		}
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