// In status_system.cpp
#include "state_system.hpp"
#include <iostream>

void StateSystem::step(float elapsed_ms)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);
    Animation& animation = registry.animations.get(player);

    animation.timer_ms += elapsed_ms;
    if (state.state == STATE::MOVE) {
        state_move();
    }
    if (state.state == STATE::ATTACK) {
        state_attack();
    }
}

void StateSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);
    Animation& animation = registry.animations.get(player);
    RenderRequest& request = registry.renderRequests.get(player);

    if (state.state != state_new && state.state != STATE::ATTACK) {
        state.state = state_new;
        animation.timer_ms = 0;
        animation.pose = 0;
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
    Animation& animation = registry.animations.get(player);
    
    if (animation.timer_ms >= PLAYER_MOVE_FRAME_DELAY) {
        RenderRequest& request = registry.renderRequests.get(player);
        animation.pose = (animation.pose + 1) % (sizeof(PLAYER_ANIMATION_MOVE) / sizeof(PLAYER_ANIMATION_MOVE[0]));
        request.used_texture = PLAYER_ANIMATION_MOVE[animation.pose];
        animation.timer_ms = 0;
    }
}

void StateSystem::state_attack()
{
    Entity player = registry.players.entities[0];
    Animation& animation = registry.animations.get(player);

    if (animation.timer_ms >= PLAYER_ATTACK_FRAME_DELAY) {
        RenderRequest& request = registry.renderRequests.get(player);
        animation.pose += 1;
        if (animation.pose != (sizeof(PLAYER_ANIMATION_ATTACK) / sizeof(PLAYER_ANIMATION_ATTACK[0]))) {
            request.used_texture = PLAYER_ANIMATION_ATTACK[animation.pose];
        }
        else {
            State& state = registry.states.get(player);
            state.state = STATE::IDLE;
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        animation.timer_ms = 0;
    }
}
