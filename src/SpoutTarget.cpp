#include "SpoutTarget.hpp"

SpoutTarget::~SpoutTarget() {
    cleanup();
}

void SpoutTarget::cleanup() {
    if (fbo) glDeleteFramebuffers(1, &fbo);
    if (texture) glDeleteTextures(1, &texture);
    fbo = 0;
    texture = 0;
    width = 0;
    height = 0;
    initialized = false;
}

void SpoutTarget::ensureSize(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (initialized && width == w && height == h && fbo != 0) return;
    
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }

    width = w;
    height = h;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D, 
        0, 
        GL_RGBA,
        width, 
        height, 
        0, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE, 
        NULL
    );

    if (!fbo) glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        texture,
        0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cleanup();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!initialized) {
        initialized = sender.CreateSender(senderName.c_str(), width, height);
    } else {
        auto update = sender.UpdateSender(senderName.c_str(), width, height);
        geode::log::warn("updated? {}", update);
    }
}

void SpoutTarget::bind() {
    if (fbo) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
    }
}

void SpoutTarget::clear() {
    if (fbo) {
        bind();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void SpoutTarget::send() {
    if (fbo && initialized) {
        sender.SendTexture(
            texture,
            GL_TEXTURE_2D,
            width,
            height,
            true,
            0
        );
    }
}
