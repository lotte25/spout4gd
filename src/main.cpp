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
    static void onModify(auto& self) {
        if (!self.setHookPriorityPre("cocos2d::CCEGLView::swapBuffers", Priority::Last)) {
            log::warn("Failed to set hook priority.");
        }
    }

    void swapBuffers() {
        if (SpoutManager::shouldSendFrame()) {
            auto size = getFrameSize();
            SpoutManager::captureScreen(size.width, size.height);
        }

        CCEGLView::swapBuffers();
    }
};
