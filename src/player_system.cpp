// In status_system.cpp
#include "player_system.hpp"
#include <iostream>
#include "animation_system.hpp"
#include "world_system.hpp"

/**
* Update the state of the player and its animation component.
* 
* @param state_new The new state.
*/
void PlayerSystem::update_state(STATE state_new)
{
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);

    if (state.state != state_new) {
        state.state = state_new;

        if (state.state == STATE::IDLE) {
            AnimationSystem::update_animation(player, PLAYER_IDLE_DURATION, PLAYER_IDLE_ANIMATION, PLAYER_IDLE_SIZE, true, false, false);
        }
        else if (state.state == STATE::MOVE) {
            AnimationSystem::update_animation(player, PLAYER_MOVE_DURATION, PLAYER_MOVE_ANIMATION, PLAYER_MOVE_SIZE, true, false, false);
        }
        else {
            AnimationSystem::update_animation(player, PLAYER_ATTACK_DURATION, PLAYER_ATTACK_ANIMATION, PLAYER_ATTACK_SIZE, false, true, false);
        }
    }
}

STATE PlayerSystem::get_state() {
    if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH || WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG) return STATE::STATE_COUNT;
    Entity player = registry.players.entities[0];
    State& state = registry.states.get(player);
    return state.state;
}
