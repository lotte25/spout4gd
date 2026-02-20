#pragma once

#include "includes.hpp"

namespace fakecursor {
    struct CursorTextureInfo {
        std::vector<uint8_t> pixels;
        int width = 0;
        int height = 0;
        float anchorX = 0.f;
        float anchorY = 0.f;
        bool success = false;
    };

    struct CursorPos {
        float x = 0.f;
        float y = 0.f;
    };

    inline std::unordered_map<std::string, GLuint> const filterMap = {
        {"Nearest", GL_NEAREST },
        {"Linear",  GL_LINEAR },
    };

    void setScale(float scale);
    void setFilter(std::string const& filter);

    void updateTextureParams();
    bool init();
    void draw(int w, int h);
    void reset();
}