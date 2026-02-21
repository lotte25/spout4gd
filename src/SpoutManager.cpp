#include "SpoutManager.hpp"
#include "ResolutionSetting.hpp"
#include "SpoutTarget.hpp"
#include "FakeCursor.hpp"

SpoutManager& SpoutManager::get() {
    static SpoutManager s_instance;
    return s_instance;
}

SpoutManager::SpoutManager() :
    frameInterval(std::chrono::nanoseconds(
        static_cast<int64_t>(1000000000.0 / 60)
    )),
    nextCaptureTime(SteadyClock::now()) {}

SpoutManager::~SpoutManager() {
    mainTarget->cleanup();
}

bool SpoutManager::validateContext() {
    HGLRC currentContext = wglGetCurrentContext();
    bool contextChanged = currentContext != lastContext;

    if (contextChanged) {
        log::warn(
            "OpenGL context changed! (old: {}, new: {})", 
            reinterpret_cast<long long>(lastContext), 
            reinterpret_cast<long long>(currentContext)
        );

        fakecursor::reset();
        mainTarget->cleanup();

        lastContext = currentContext;
        return false;
    }

    return true;
}

bool SpoutManager::shouldSendFrame() {
    auto now = SteadyClock::now();
    if (now >= nextCaptureTime) {
        nextCaptureTime += frameInterval;

        if (now > nextCaptureTime + frameInterval) {
            nextCaptureTime = now + frameInterval;
        }

        return true;
    }
    return false;
}

void SpoutManager::captureScreen(int w, int h, bool cursorHidden) {
    if (w <= 0 || h <= 0) return;
    if (!validateContext()) return;

    int targetWidth = w;
    int targetHeight = h;

    if (resolution.enabled && resolution.width > 0 && resolution.height > 0) {
        targetWidth = resolution.width;
        targetHeight = resolution.height;
    }

    mainTarget->ensureSize(targetWidth, targetHeight);

    GLint oldDrawFBO;
    GLint oldReadFBO;
    GLint oldTexture;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainTarget->fbo);
    glReadBuffer(GL_BACK);

    GLenum filter = (w == targetWidth && h == targetHeight) ? GL_NEAREST : GL_LINEAR;

    glBlitFramebuffer(
        0, 0, w, h,
        0, 0, targetWidth, targetHeight,
        GL_COLOR_BUFFER_BIT,
        filter
    );

    if (cursorEnabled && !cursorHidden) {
        if (!fakecursor::init()) {
            log::warn("Couldn't create fake cursor");
            return;
        }

        fakecursor::draw(w, h, targetWidth, targetHeight);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
    glBindTexture(GL_TEXTURE_2D, oldTexture);

    mainTarget->send();
}

void SpoutManager::updateFrameInterval(int fps) {
    frameInterval = std::chrono::nanoseconds(
        static_cast<int64_t>(1000000000.0 / fps)
    );

    nextCaptureTime = SteadyClock::now() + frameInterval;

    log::info(
        "frame interval updated to {} fps ({}ns)", 
        fps, frameInterval.count()
    );
}

void SpoutManager::setOutputResolution(CustomResolution const& res) {
    resolution = res;
}

void SpoutManager::enableCursor(bool show) {
    cursorEnabled = show;
}