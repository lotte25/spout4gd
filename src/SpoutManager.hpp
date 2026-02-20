#pragma once

#include "includes.hpp"
#include "SpoutTarget.hpp"
#include <chrono>

class SpoutManager {
public:
    static SpoutManager& get();

    bool validateContext();
    bool shouldSendFrame();
    void drawCursor(int w, int h);
    void captureScreen(int w, int h);
    void updateFrameInterval(int fps);
    void setCursorVisible(bool show);
    void reset();

private:
    SpoutManager();
    ~SpoutManager();

    HGLRC lastContext;
    /* 
        I need to explain this one i think.
        If i initialize SpoutTarget in the stack, it will be freed but
        late enough for the dx11 device to not exist (i think)
        So if i do SpoutTarget mainTarget{"whatever"}, it will crash the game on exit.
        I release resources in SpoutTarget::cleanup, which is enough to not crash and
        not leave resources open. 
        Maybe there is a better way or a fix to this crash, but i don't know about it for now.
    */
    SpoutTarget* mainTarget = new SpoutTarget("Spout4GD - Screen");
    using SteadyClock = std::chrono::steady_clock;
    std::chrono::nanoseconds frameInterval;
    SteadyClock::time_point nextCaptureTime;
    bool shouldRenderCursor = true;
};