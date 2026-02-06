#include "SpoutManager.hpp"
#include "SpoutTarget.hpp"

namespace SpoutManager {
    static SpoutTarget mainTarget("Geometry Dash - Screen");
    // SpoutTarget playerTarget("Geometry Dash - Player");

/*  SimplePlayer* playerClone = nullptr;
    int lastIconID = -1;
    IconType lastIconType = IconType::Cube;
    int lastCol1 = -1;
    int lastCol2 = -1;
    const int PLAYER_RES = 512; */

    using clock = std::chrono::steady_clock;
    using duration = std::chrono::duration<double>;

    std::chrono::nanoseconds frameInterval(static_cast<int64_t>(1000000000.0 / 60));
    clock::time_point nextCaptureTime = clock::now();

    bool shouldSendFrame() {
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

    void captureScreen(int w, int h) {
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

        glBindFramebuffer(GL_READ_FRAMEBUFFER, oldReadFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldDrawFBO);
        glBindTexture(GL_TEXTURE_2D, oldTexture);

        mainTarget.send();
    }

/*     void capturePlayer() {
        auto pl = PlayLayer::get();
        if (!pl || !pl->m_player1) return;
        auto player = pl->m_player1;
        auto gm = GameManager::sharedState();

        playerTarget.ensureSize(PLAYER_RES, PLAYER_RES);

        IconType currentType = IconType::Cube;
        int iconID = gm->getPlayerFrame();

        if (player->m_isShip) {
            currentType = IconType::Ship;
            iconID = gm->getPlayerShip();
        }
        else if (player->m_isBall) {
            currentType = IconType::Ball;
            iconID = gm->getPlayerBall();
        }
        else if (player->m_isBird) {
            currentType = IconType::Ufo;
            iconID = gm->getPlayerBird();
        }
        else if (player->m_isDart) {
            currentType = IconType::Wave;
            iconID = gm->getPlayerDart();
        }
        else if (player->m_isRobot) {
            currentType = IconType::Robot;
            iconID = gm->getPlayerRobot();
        }
        else if (player->m_isSpider) {
            currentType = IconType::Spider;
            iconID = gm->getPlayerSpider();
        }
        else if (player->m_isSwing) {
            currentType = IconType::Swing;
            iconID = gm->getPlayerSwing();
        }

        int col1 = gm->getPlayerColor();
        int col2 = gm->getPlayerColor2();

        if (!playerClone) {
            playerClone = SimplePlayer::create(iconID);
            playerClone->retain();
            playerClone->setAnchorPoint({ 0.5f, 0.5f });
        }

        if (iconID != lastIconID || currentType != lastIconType) {
            playerClone->updatePlayerFrame(iconID, currentType);
            lastIconID = iconID;
            lastIconType = currentType;
        }

        if (col1 != lastCol1 || col2 != lastCol2) {
            playerClone->setColor(gm->colorForIdx(col1));
            playerClone->setSecondColor(gm->colorForIdx(col2));
            lastCol1 = col1;
            lastCol2 = col2;
        }

        playerClone->updateColors();

        playerClone->setRotation(player->getRotation());
        playerClone->setOpacity(255);
        playerClone->setScale(3.f);

        playerClone->setPosition(ccp(PLAYER_RES / 2.0f, PLAYER_RES / 2.0f));

        if (gm->getPlayerGlow()) {
            playerClone->setGlowOutline(gm->colorForIdx(col1)); // O el color de glow que corresponda
        } else {
            playerClone->disableGlowOutline();
        }

        GLint oldFBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);
        GLint oldViewport[4];
        glGetIntegerv(GL_VIEWPORT, oldViewport);

        glBindFramebuffer(GL_FRAMEBUFFER, playerTarget.fbo);
        glViewport(0, 0, PLAYER_RES, PLAYER_RES);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        kmGLMatrixMode(KM_GL_PROJECTION);
        kmGLPushMatrix();
        kmGLLoadIdentity();

        kmMat4 orthoMatrix;
        kmMat4OrthographicProjection(&orthoMatrix, 0, PLAYER_RES, 0, PLAYER_RES, -1024, 1024);
        kmGLMultMatrix(&orthoMatrix);

        kmGLMatrixMode(KM_GL_MODELVIEW);
        kmGLPushMatrix();
        kmGLLoadIdentity();

        playerClone->visit();

        kmGLMatrixMode(KM_GL_MODELVIEW);
        kmGLPopMatrix();

        kmGLMatrixMode(KM_GL_PROJECTION);
        kmGLPopMatrix();

        glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
        glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

        playerTarget.send();
    } */

    void reset() {
        mainTarget.cleanup();

        geode::log::warn("context reset");
    }   

    void updateFrameInterval(double fps) {
        frameInterval = std::chrono::nanoseconds(
            static_cast<int64_t>(1000000000.0 / fps)
        );

        nextCaptureTime = clock::now() + frameInterval;

        log::info("frame interval updated to {} fps ({}ns)", fps, frameInterval.count());
    }
}
