#include "includes.hpp"
#include "SpoutManager.hpp"
#include "FakeCursor.hpp"
#include <Geode/modify/CCEGLView.hpp>

$on_mod(Loaded) {
    auto savedFPS = geode::Mod::get()->getSettingValue<int64_t>("output-fps");
    auto showCursor = geode::Mod::get()->getSettingValue<bool>("show-cursor");
    auto cursorScale = geode::Mod::get()->getSettingValue<double>("cursor-scale");
    auto cursorFilter = geode::Mod::get()->getSettingValue<std::string>("cursor-filter");

    SpoutManager::get().updateFrameInterval(savedFPS);
    SpoutManager::get().setCursorVisible(showCursor);
    FakeCursor::setScale(cursorScale);
    FakeCursor::setFilter(cursorFilter);

    listenForSettingChanges("output-fps", [](int fps) {
        SpoutManager::get().updateFrameInterval(fps);
    });

    listenForSettingChanges("show-cursor", [](bool show) {
        SpoutManager::get().setCursorVisible(show);
    });

    listenForSettingChanges("cursor-scale", [](float scale) {
        FakeCursor::setScale(scale);
    });

    listenForSettingChanges("cursor-filter", [](std::string const& filter) {
        FakeCursor::setFilter(filter);
    });
}

class $modify(CCEGLView) {
    static void onModify(auto& self) {
        if (!self.setHookPriorityPre("cocos2d::CCEGLView::swapBuffers", Priority::Last)) {
            log::warn("Failed to set hook priority.");
        }
    }

    void swapBuffers() {
        if (SpoutManager::get().shouldSendFrame()) {
            auto size = getFrameSize();
            SpoutManager::get().captureScreen(size.width, size.height);
        }
        CCEGLView::swapBuffers();
    }
};