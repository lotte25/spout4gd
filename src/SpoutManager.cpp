#include "SpoutManager.hpp"
#include "SpoutTarget.hpp"

float scale = 1.3f;

SpoutManager& SpoutManager::get() {
    static SpoutManager instance;
    return instance;
}

SpoutManager::SpoutManager() : 
    mainTarget("Spout4GD - Screen"), 
    frameInterval(std::chrono::nanoseconds(static_cast<int64_t>(1000000000.0 / 60))),
    nextCaptureTime(clock::now()) {}

SpoutManager::~SpoutManager() {}

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

void SpoutManager::drawCursor(int w, int h, float scale) {
    CCTexture2D* texture = nullptr;
    // Load cursor sprite to texture cache
    if (cursorTextureID == 0) {
        texture = CCTextureCache::sharedTextureCache()->addImage("cursor.png"_spr, false);
        if (texture) {
            cursorTextureID = texture->getName();
            ccTexParams params = {GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE};
            texture->setTexParameters(&params);
            
            log::info("Cursor texture ID: {}", cursorTextureID);
        } else {
            log::error("Couldnt load cursor texture");
            return;
        }
    }

    auto mousePos = CCEGLView::sharedOpenGLView()->getMousePosition();

    float mouseX = mousePos.x;
    float mouseY = h - mousePos.y; 
    float mouseW = 32.0f;
    float mouseH = 32.0f;

    if (texture) {
        mouseW = texture->getContentSize().width * scale;
        mouseH = texture->getContentSize().height * scale;
    }

    // Save current attributes
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // Save shader program
    GLint oldProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    glUseProgram(0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0); 

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // Start drawing the texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, cursorTextureID);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); 
        glVertex2f(mouseX, mouseY); 

        glTexCoord2f(1.0f, 0.0f); 
        glVertex2f(mouseX + mouseW, mouseY);

        glTexCoord2f(1.0f, 1.0f); 
        glVertex2f(mouseX + mouseW, mouseY - mouseH);

        glTexCoord2f(0.0f, 1.0f); 
        glVertex2f(mouseX, mouseY - mouseH);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    // Restore matrix stack
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    // Restore all previous attributes
    glPopAttrib();
    // Restore shader program
    glUseProgram(oldProgram);
}

void SpoutManager::captureScreen(int w, int h) {
    if (!w || !h) return;

    mainTarget.ensureSize(w, h);

    GLint oldDrawFBO, oldReadFBO, oldTexture;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldDrawFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &oldReadFBO);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mainTarget.fbo);
    glReadBuffer(GL_BACK);

    glBlitFramebuffer(
        0, 0, w, h,
        0, 0, w, h,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST
    );

    if (shouldRenderCursor) drawCursor(w, h, scale);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
    glBindTexture(GL_TEXTURE_2D, oldTexture);

    mainTarget.send();
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