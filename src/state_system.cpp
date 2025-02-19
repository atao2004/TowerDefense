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
    else {
        state_attack();
    }
}

void StateSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);
    State& state = registry.states.get(player);

    if (state.state != state_new && state.state != STATE::ATTACK) {
        state.state = state_new;
        timer_ms = 0;
        if (state.state == STATE::IDLE) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        else if (state.state == STATE::MOVE) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
        }
        else {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_ACTION_1;
        }
    }
}

void StateSystem::state_move()
{
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);

    if (timer_ms >= PLAYER_MOVE_FRAME_DELAY) {
        if (request.used_texture == TEXTURE_ASSET_ID::PLAYER_WALK_1) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_2;
        }
        else {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_WALK_1;
        }
        timer_ms = 0;
    }
}

void StateSystem::state_attack()
{
    Entity player = registry.players.entities[0];
    RenderRequest& request = registry.renderRequests.get(player);

    if (timer_ms >= PLAYER_ATTACK_FRAME_DELAY) {
        if (request.used_texture == TEXTURE_ASSET_ID::PLAYER_ACTION_1) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_ACTION_2;
        }
        else {
            State& state = registry.states.get(player);
            state.state = STATE::IDLE;
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        timer_ms = 0;
    }
}
