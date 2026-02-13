#include "includes.hpp"
#include "SpoutManager.hpp"
#include <Geode/modify/CCEGLView.hpp>

SpoutManager& manager = SpoutManager::get();

$on_mod(Loaded) {
    auto savedFPS = geode::Mod::get()->getSavedValue("output-fps", 60);
    auto showCursor = geode::Mod::get()->getSavedValue("show-cursor", true);
    auto cursorScale = geode::Mod::get()->getSavedValue("cursor-scale", 1.f);

    manager.updateFrameInterval(savedFPS);
    manager.setCursorVisible(showCursor);
    manager.setCursorScale(cursorScale);

    listenForSettingChanges("output-fps", [](int fps) {
        manager.updateFrameInterval(fps);
    });

    listenForSettingChanges("show-cursor", [](bool show) {
        manager.setCursorVisible(show);
    });

    listenForSettingChanges("cursor-scale", [](float scale) {
        manager.setCursorScale(scale);
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