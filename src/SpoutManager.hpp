#pragma once

#include "includes.hpp"

namespace SpoutManager {
    bool shouldSendFrame();
    void captureScreen(int w, int h);
    // void capturePlayer();
    void reset();
    void updateFrameInterval(double fps);
}