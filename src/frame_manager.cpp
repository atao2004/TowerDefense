#include "frame_manager.hpp"
#include <iostream>

FrameManager::FrameManager()
{
	frame = 0;
}

void FrameManager::tick()
{
	frame++;
}

bool FrameManager::can_update(int frameInterval)
{
	return frame % frameInterval == 0;
}
