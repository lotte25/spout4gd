#pragma once

#include "includes.hpp"

class SpoutTarget {
public:
    SpoutTarget(std::string const& name);
    ~SpoutTarget();

    SpoutTarget(SpoutTarget const&) = delete;
    SpoutTarget& operator=(SpoutTarget const&) = delete;

    void cleanup();
    void ensureSize(int w, int h);
    void bind(bool read = false);
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