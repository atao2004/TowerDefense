#include <iostream>
#include "world_system.hpp"
#include "movement_system.hpp"

MovementSystem::MovementSystem()
{

}

MovementSystem::~MovementSystem()
{

}

void MovementSystem::step(float elapsed_ms)
{
    checkBoundaries(elapsed_ms);
}

void MovementSystem::checkBoundaries(float elapsed_ms) {
    // Player movement
	Entity player = registry.players.entities[0];
	Motion &player_motion = registry.motions.get(player);

    if (player_motion.position.x < PLAYER_LEFT_BOUNDARY && player_motion.velocity.x < 0) {
        for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x = 0;
        }
    }

    if (player_motion.position.x > (WINDOW_WIDTH_PX - PLAYER_LEFT_BOUNDARY) && player_motion.velocity.x > 0) {
        for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.x = 0;
        }
    }

    if (player_motion.position.y > (WINDOW_HEIGHT_PX - PLAYER_UP_BOUNDARY) && player_motion.velocity.y > 0) {
        for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y = 0;
        }
    }

    if (player_motion.position.y < PLAYER_UP_BOUNDARY && player_motion.velocity.y < 0) {
        for (Entity mwc_entity : registry.moveWithCameras.entities) {
            if (registry.motions.has(mwc_entity)) registry.motions.get(mwc_entity).velocity.y = 0;
        }
    }
}