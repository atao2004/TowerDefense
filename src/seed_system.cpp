#include "seed_system.hpp"
#include "world_init.hpp"
#include "world_system.hpp"
#include <iostream>

RenderSystem* SeedSystem::renderer;

void SeedSystem::init(RenderSystem* renderer_arg)
{
	renderer = renderer_arg;
}

void SeedSystem::step(float elapsed_ms)
{
	for (Entity i : registry.seeds.entities)
	{

		if (!registry.moveWithCameras.has(i))
		{
			if (registry.seeds.get(i).timer <= 0)
			{
				vec2 pos;
				pos.x = registry.motions.get(i).position.x;
				pos.y = registry.motions.get(i).position.y;
				registry.remove_all_components_of(i);
				registry.seeds.remove(i);
				createTower(renderer, { pos.x - GRID_CELL_WIDTH_PX / 2, pos.y - GRID_CELL_HEIGHT_PX / 2 });
				
				if (registry.screenStates.components[0].cg_index++ == 11)
					return WorldSystem::start_cg(renderer);
			}
			else
			{
				registry.seeds.get(i).timer -= elapsed_ms;
			}
		}
	}
}
