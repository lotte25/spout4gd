#include "SpoutTarget.hpp"

SpoutTarget::SpoutTarget(std::string const& name) : senderName(name) {}

SpoutTarget::~SpoutTarget() {
    cleanup();
}

void SpoutTarget::cleanup() {
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    initialized = false;
    sender.ReleaseSender();
    sender.CloseOpenGL();
}

void SpoutTarget::ensureSize(int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (initialized && width == w && height == h) return;

    log::debug("recreating resources");

    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
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
        nullptr
    );

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        texture,
        0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log::warn("incomplete framebuffer, cleaning up");
        cleanup();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (!initialized) {
        initialized = sender.CreateSender(senderName.c_str(), width, height);
    } else {
        sender.UpdateSender(senderName.c_str(), width, height);
    }
}

void SpoutTarget::bind(bool read) {
    if (fbo) {
        glBindFramebuffer(read ? GL_READ_FRAMEBUFFER : GL_DRAW_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
    }
}

void SpoutTarget::clear() {
    if (fbo) {
        bind();
        glClearColor(0.f, 0.f, 0.f, 0.f);
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
