#include <iostream>
#include "world_system.hpp"
#include "movement_system.hpp"

MovementSystem::MovementSystem()
{
}

MovementSystem::~MovementSystem()
{
}

void MovementSystem::step(float elapsed_ms, GAME_SCREEN_ID game_screen)
{
    checkBoundaries(elapsed_ms, game_screen);
    checkVelocities(elapsed_ms);
}

void MovementSystem::checkBoundaries(float elapsed_ms, GAME_SCREEN_ID game_screen)
{
    // Player movement
    Entity player = registry.players.entities[0];
    Motion &player_motion = registry.motions.get(player);

    if (game_screen == GAME_SCREEN_ID::TUTORIAL)
    {
        if (player_motion.position.x < PLAYER_LEFT_BOUNDARY && player_motion.velocity.x < 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.x = 0;
            }
        }

        if (player_motion.position.x > PLAYER_RIGHT_BOUNDARY_TUTORIAL && player_motion.velocity.x > 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.x = 0;
            }
        }

        if (player_motion.position.y > PLAYER_DOWN_BOUNDARY_TUTORIAL && player_motion.velocity.y > 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.y = 0;
            }
        }

        if (player_motion.position.y < PLAYER_UP_BOUNDARY && player_motion.velocity.y < 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.y = 0;
            }
        }
    }
    else
    {
        if (player_motion.position.x < PLAYER_LEFT_BOUNDARY && player_motion.velocity.x < 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.x = 0;
            }
        }

        if (player_motion.position.x > PLAYER_RIGHT_BOUNDARY && player_motion.velocity.x > 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.x = 0;
            }
        }

        if (player_motion.position.y > PLAYER_DOWN_BOUNDARY && player_motion.velocity.y > 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.y = 0;
            }
        }

        if (player_motion.position.y < PLAYER_UP_BOUNDARY && player_motion.velocity.y < 0)
        {
            for (Entity mwc_entity : registry.moveWithCameras.entities)
            {
                if (registry.motions.has(mwc_entity))
                    registry.motions.get(mwc_entity).velocity.y = 0;
            }
        }
    }
}

void MovementSystem::checkVelocities(float elapsed_ms)
{
    // Player movement
    Entity player = registry.players.entities[0];
    Motion &player_motion = registry.motions.get(player);
    bool is_dashing = WorldSystem::player_is_dashing;

    float max_right_speed = is_dashing ? PLAYER_MOVE_RIGHT_SPEED * PLAYER_DASH_SPEED_MULTIPLIER : PLAYER_MOVE_RIGHT_SPEED;
    float max_left_speed = is_dashing ? PLAYER_MOVE_LEFT_SPEED * PLAYER_DASH_SPEED_MULTIPLIER : PLAYER_MOVE_LEFT_SPEED;
    float max_down_speed = is_dashing ? PLAYER_MOVE_DOWN_SPEED * PLAYER_DASH_SPEED_MULTIPLIER : PLAYER_MOVE_DOWN_SPEED;
    float max_up_speed = is_dashing ? PLAYER_MOVE_UP_SPEED * PLAYER_DASH_SPEED_MULTIPLIER : PLAYER_MOVE_UP_SPEED;

    if (player_motion.velocity.x > PLAYER_MOVE_RIGHT_SPEED)
    {
        for (Entity mwc_entity : registry.moveWithCameras.entities)
        {
            if (registry.motions.has(mwc_entity))
                registry.motions.get(mwc_entity).velocity.x = max_right_speed;
        }
    }

    if (player_motion.velocity.x < PLAYER_MOVE_LEFT_SPEED)
    {
        for (Entity mwc_entity : registry.moveWithCameras.entities)
        {
            if (registry.motions.has(mwc_entity))
                registry.motions.get(mwc_entity).velocity.x = max_left_speed;
        }
    }

    if (player_motion.velocity.y > PLAYER_MOVE_DOWN_SPEED)
    {
        for (Entity mwc_entity : registry.moveWithCameras.entities)
        {
            if (registry.motions.has(mwc_entity))
                registry.motions.get(mwc_entity).velocity.y = max_down_speed;
        }
    }

    if (player_motion.velocity.y < PLAYER_MOVE_UP_SPEED)
    {
        for (Entity mwc_entity : registry.moveWithCameras.entities)
        {
            if (registry.motions.has(mwc_entity))
                registry.motions.get(mwc_entity).velocity.y = max_up_speed;
        }
    }
}