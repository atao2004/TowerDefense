// In status_system.cpp
#include "state_system.hpp"
#include <iostream>

void StateSystem::init()
{
    Entity player = registry.players.entities[0];
    request = &registry.renderRequests.get(player);
    state = &registry.states.get(player);
}

void StateSystem::step(float elapsed_ms)
{
    if (state->state == STATE::IDLE) {
        request->used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
    } else {
        request->used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
    }
}
