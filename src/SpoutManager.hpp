#pragma once

#include "includes.hpp"
#include "SpoutTarget.hpp"
#include <chrono>

class SpoutManager {
public:
    static SpoutManager& get();

    bool shouldSendFrame();
    void drawCursor(int w, int h, float scale);
    void captureScreen(int w, int h);
    void updateFrameInterval(double fps);
    void setCursorVisible(bool show);
    void reset();

private:
    SpoutManager();
    ~SpoutManager();

    SpoutTarget mainTarget;
    using clock = std::chrono::steady_clock;
    std::chrono::nanoseconds frameInterval;
    clock::time_point nextCaptureTime;
    GLuint cursorTextureID = 0;

    bool shouldRenderCursor = true;
};