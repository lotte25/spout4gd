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

    static const std::unordered_map<std::string, GLuint> filterMap = {
        { "Nearest", GL_NEAREST },
        { "Linear",  GL_LINEAR },
    };

    CursorTextureInfo CreateTexture();

    void setScale(float scale);
    void setFilter(const std::string& filter);

    void updateTextureParams();
    bool init();
    void draw(int w, int h);
}