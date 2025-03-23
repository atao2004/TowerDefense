#pragma once

#include <vector>

class FrameManager {
public:
    static void tick(float elapsed_ms);
    FrameManager(int frameInterval);
    bool can_update();
    float get_time();
private:
    static std::vector<FrameManager*> frame_managers;
    static unsigned int frame;
    int frameInterval;
    float time;
};
