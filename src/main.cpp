#include "includes.hpp"
#include "SpoutManager.hpp"
#include <Geode/modify/CCEGLView.hpp>

SpoutManager& manager = SpoutManager::get();

$on_mod(Loaded) {
    auto savedFPS = geode::Mod::get()->getSavedValue("output-fps", 60);
    manager.updateFrameInterval(savedFPS);

    listenForSettingChanges("output-fps", [](int fps) {
        manager.updateFrameInterval(fps);
    });
}

class $modify(CCEGLView) {
    static void onModify(auto& self) {
        if (!self.setHookPriorityPre("cocos2d::CCEGLView::swapBuffers", Priority::Last)) {
            log::warn("Failed to set hook priority.");
        }
    }

    void swapBuffers() {
        if (manager.shouldSendFrame()) {
            auto size = getFrameSize();
            manager.captureScreen(size.width, size.height);
        }
        CCEGLView::swapBuffers();
    }
};