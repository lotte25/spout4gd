#pragma once

#include "includes.hpp"

namespace FakeCursor {
    struct CursorTextureInfo {
        std::vector<uint8_t> pixels;
        int width = 0;
        int height = 0;
        float anchorX = 0.f;
        float anchorY = 0.f;
        bool success = false;
    };

    struct CursorPos {
        float x;
        float y;
    };

    CursorTextureInfo CreateTexture();

    void setScale(float scale);
    bool init();
    void draw(int w, int h);
}