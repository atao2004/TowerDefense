// In status_system.cpp
#include "state_system.hpp"
#include <iostream>

void StateSystem::step(float elapsed_ms)
{
    //State Idle : Update player texture(idle)
    //State Walk : Update player texture(walk1 / walk2)
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);
    State& state = registry.states.get(player);
    if (state.state == STATE::IDLE) {
        request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
    } else {
        request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
    }
}
