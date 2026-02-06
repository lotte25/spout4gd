#include "includes.hpp"
#include <Geode/modify/CCEGLView.hpp>
#include "SpoutManager.hpp"

$on_mod(Loaded) {
    auto savedFPS = geode::Mod::get()->getSavedValue("output-fps", 60);
    SpoutManager::updateFrameInterval(savedFPS);

    listenForSettingChanges("output-fps", [](int fps) {
        SpoutManager::updateFrameInterval(fps);
    });
}

class $modify(CCEGLView) {
    void swapBuffers() {
        if (SpoutManager::shouldSendFrame()) {
            auto size = getFrameSize();
            auto w = static_cast<int>(size.width);
            auto h = static_cast<int>(size.height);
            SpoutManager::captureScreen(w, h);
            /*if (PlayLayer::get()) {
                SpoutHandler::capturePlayer();
            } */
        }

        CCEGLView::swapBuffers();
    }
};
