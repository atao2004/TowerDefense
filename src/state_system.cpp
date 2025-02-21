// In status_system.cpp
#include "state_system.hpp"
#include <iostream>

void StateSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);
    RenderRequest& request = registry.renderRequests.get(player);

    if (state.state != state_new && state.state != STATE::ATTACK) {
        state.state = state_new;

        if (registry.animations.has(player)) {
            registry.animations.remove(player);
        }

        if (state.state == STATE::IDLE) {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        else if (state.state == STATE::MOVE) {
            Animation& animation = registry.animations.emplace(player);
            animation.transition_ms = PLAYER_MOVE_FRAME_DELAY;
            animation.pose_count = sizeof(PLAYER_ANIMATION_MOVE) / sizeof(PLAYER_ANIMATION_MOVE[0]);
            animation.textures = PLAYER_ANIMATION_MOVE;
            request.used_texture = PLAYER_ANIMATION_MOVE[0];
        }
        else {
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_ACTION_1;
        }
    }
}

void StateSystem::state_attack()
{
    //Entity player = registry.players.entities[0];
    //Animation& animation = registry.animations.get(player);

    //if (animation.timer_ms >= PLAYER_ATTACK_FRAME_DELAY) {
    //    RenderRequest& request = registry.renderRequests.get(player);
    //    animation.pose += 1;
    //    if (animation.pose != (sizeof(PLAYER_ANIMATION_ATTACK) / sizeof(PLAYER_ANIMATION_ATTACK[0]))) {
    //        request.used_texture = PLAYER_ANIMATION_ATTACK[animation.pose];
    //    }
    //    else {
    //        State& state = registry.states.get(player);
    //        state.state = STATE::IDLE;
    //        request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
    //    }
    //    animation.timer_ms = 0;
    //}
}
