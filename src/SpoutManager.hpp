#pragma once

#include "includes.hpp"
#include "SpoutTarget.hpp"
#include <chrono>

class SpoutManager {
public:
    static SpoutManager& get();

    bool shouldSendFrame();
    void drawCursor(int w, int h);
    void captureScreen(int w, int h);
    void updateFrameInterval(double fps);
    void setCursorVisible(bool show);
    void setCursorScale(float scale);
    void reset();

private:
    SpoutManager();
    ~SpoutManager();

    SpoutTarget* mainTarget = new SpoutTarget("Spout4GD - Screen");
    using clock = std::chrono::steady_clock;
    std::chrono::nanoseconds frameInterval;
    clock::time_point nextCaptureTime;
    // Cursor
    bool shouldRenderCursor = true;
    float cursorScale = 1.f;
};