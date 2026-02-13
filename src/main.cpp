#include "includes.hpp"
#include "SpoutManager.hpp"
#include "FakeCursor.hpp"
#include <Geode/modify/CCEGLView.hpp>

SpoutManager& manager = SpoutManager::get();

$on_mod(Loaded) {
    auto savedFPS = geode::Mod::get()->getSettingValue<int64_t>("output-fps");
    auto showCursor = geode::Mod::get()->getSettingValue<bool>("show-cursor");
    auto cursorScale = geode::Mod::get()->getSettingValue<double>("cursor-scale");

    manager.updateFrameInterval(savedFPS);
    manager.setCursorVisible(showCursor);
    FakeCursor::setScale(cursorScale);

    listenForSettingChanges("output-fps", [](int fps) {
        manager.updateFrameInterval(fps);
    });

    listenForSettingChanges("show-cursor", [](bool show) {
        manager.setCursorVisible(show);
    });

    listenForSettingChanges("cursor-scale", [](float scale) {
        FakeCursor::setScale(scale);
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