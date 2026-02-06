#pragma once

#include "includes.hpp"

struct SpoutTarget {
    bool initialized = false;
    SpoutSender sender;
    GLuint fbo = 0;
    GLuint texture = 0;
    int width = 0;
    int height = 0;
    std::string senderName;

    SpoutTarget(std::string name) : senderName(name) {}
    ~SpoutTarget();
    void cleanup();
    void ensureSize(int w, int h);
    void bind();
    void clear();
    void send();
};