#pragma once

#include "includes.hpp"

class SpoutTarget {
public:
    SpoutTarget(const std::string& name);
    ~SpoutTarget();

    void cleanup();
    void ensureSize(int w, int h);
    void bind();
    void clear();
    void send();

    bool initialized = false;
    std::string senderName;
    GLuint fbo = 0;
private:
    SpoutSender sender;
    GLuint texture = 0;
    int width = 0;
    int height = 0;
};