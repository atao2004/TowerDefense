#include "screen_system.hpp"
#include <iostream>

void ScreenSystem::step(float elapsed_ms)
{
	Player& player = registry.players.components[0];
	ScreenState& screen = registry.screenStates.components[0];
	screen.hp_percentage = player.health / player.health_max;
}
