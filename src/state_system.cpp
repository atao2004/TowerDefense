// In status_system.cpp
#include "state_system.hpp"
#include <iostream>
#include "animation_system.hpp"

/**
* Update the state of the player and its animation component.
* 
* @param state_new The new state.
*/
void StateSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);
    RenderRequest& request = registry.renderRequests.get(player);

    if (state.state != state_new) {
        state.state = state_new;

        if (state.state == STATE::IDLE) {
            if (registry.animations.has(player)) {
                registry.animations.remove(player);
            }
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        else if (state.state == STATE::MOVE) {
            AnimationSystem::update_animation(player, PLAYER_MOVE_FRAME_DELAY, PLAYER_ANIMATION_MOVE, sizeof(PLAYER_ANIMATION_MOVE) / sizeof(PLAYER_ANIMATION_MOVE[0]), true, false);
        }
        else {
            AnimationSystem::update_animation(player, PLAYER_ATTACK_FRAME_DELAY, PLAYER_ANIMATION_ATTACK, sizeof(PLAYER_ANIMATION_ATTACK) / sizeof(PLAYER_ANIMATION_ATTACK[0]), false, true);
        }
    }
}
