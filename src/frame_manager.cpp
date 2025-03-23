#include "frame_manager.hpp"
#include <iostream>

std::vector<FrameManager*> FrameManager::frame_managers;
unsigned int FrameManager::frame = 0;

FrameManager::FrameManager(int frameInterval)
{
	frame_managers.push_back(this);
	this->frameInterval = frameInterval;
	time = 0;
}

void FrameManager::tick(float elapsed_ms)
{
	frame++;
	for (FrameManager* frame_manager : frame_managers) {
		frame_manager->time += elapsed_ms;
	}
}

bool FrameManager::can_update()
{
	return frame % frameInterval == 0;
}

float FrameManager::get_time()
{
	float elapsed_time = time;
	time = 0;
	return elapsed_time;
}
