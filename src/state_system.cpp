// In status_system.cpp
#include "state_system.hpp"
#include <iostream>

float StateSystem::timer_ms = 0;

void StateSystem::step(float elapsed_ms)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);

    timer_ms += elapsed_ms;
    if (state.state == STATE::MOVE) {
        state_move();
    }
}

void StateSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);
    State& state = registry.states.get(player);

    if (state.state != state_new) {
        state.state = state_new;
        timer_ms = 0;
        if (state.state == STATE::IDLE) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        else {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
        }
    }
}

void StateSystem::state_move()
{
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);

    if (timer_ms >= 1000) {
        if (request.used_texture == TEXTURE_ASSET_ID::PLAYER_WALK_1) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_2;
        }
        else {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
        }
        timer_ms = 0;
    }
}
