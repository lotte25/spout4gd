#include "SpoutManager.hpp"
#include "SpoutTarget.hpp"
#include "FakeCursor.hpp"

SpoutManager& SpoutManager::get() {
    static SpoutManager instance;
    return instance;
}

SpoutManager::SpoutManager() :
    frameInterval(std::chrono::nanoseconds(static_cast<int64_t>(1000000000.0 / 60))),
    nextCaptureTime(clock::now()) {}

SpoutManager::~SpoutManager() {
    mainTarget->cleanup();
}

bool SpoutManager::shouldSendFrame() {
    auto now = clock::now();
    if (now >= nextCaptureTime) {
        nextCaptureTime += frameInterval;

        if (now > nextCaptureTime + frameInterval) {
            nextCaptureTime = now + frameInterval;
        }

        return true;
    }
    return false;
}

void SpoutManager::drawCursor(int w, int h) {
    if (!FakeCursor::init()) {
        log::warn("Couldn't create fake cursor");
        return;
    }

    FakeCursor::draw(w, h);
}

void SpoutManager::captureScreen(int w, int h) {
    if (!w || !h) return;

    mainTarget->ensureSize(w, h);

    GLint oldDrawFBO, oldReadFBO, oldTexture;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainTarget->fbo);
    glReadBuffer(GL_BACK);

    glBlitFramebuffer(
        0, 0, w, h,
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    if (shouldRenderCursor) drawCursor(w, h);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
    glBindTexture(GL_TEXTURE_2D, oldTexture);

    mainTarget->send();
}

void SpoutManager::updateFrameInterval(double fps) {
    frameInterval = std::chrono::nanoseconds(
        static_cast<int64_t>(1000000000.0 / fps)
    );

    nextCaptureTime = clock::now() + frameInterval;

    log::info("frame interval updated to {} fps ({}ns)", fps, frameInterval.count());
}

void SpoutManager::setCursorVisible(bool show) {
    shouldRenderCursor = show;
}