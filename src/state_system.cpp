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

    if (state.state != state_new) {
        state.state = state_new;

        if (state.state == STATE::IDLE) {
            RenderRequest& request = registry.renderRequests.get(player);
            request.used_texture = TEXTURE_ASSET_ID::PLAYER_IDLE;
        }
        else if (state.state == STATE::MOVE) {
            AnimationSystem::update_animation(player, PLAYER_MOVE_DURATION, PLAYER_MOVE_ANIMATION, PLAYER_MOVE_SIZE, true, false, false);
        }
        else {
            AnimationSystem::update_animation(player, PLAYER_ATTACK_DURATION, PLAYER_ATTACK_ANIMATION, PLAYER_ATTACK_SIZE, false, true, false);
        }
    }
}
