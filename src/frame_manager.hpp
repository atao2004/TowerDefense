#pragma once

class FrameManager {
public:
    FrameManager();
    void tick();
    bool can_update(int frameInterval);
private:
    unsigned int frame = 0;
};
