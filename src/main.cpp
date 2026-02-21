#include "includes.hpp"
#include "SpoutManager.hpp"
#include "FakeCursor.hpp"
#include "ResolutionSetting.hpp"
#include <Geode/modify/CCEGLView.hpp>

$execute {
    (void)geode::Mod::get()->registerCustomSettingType("resolution-type", &ResolutionSettingV3::parse);

    auto savedFPS = geode::Mod::get()->getSettingValue<int64_t>("output-fps");
    auto savedResolution = geode::Mod::get()->getSettingValue<CustomResolution>("output-resolution");
    auto showCursor = geode::Mod::get()->getSettingValue<bool>("show-cursor");
    auto cursorScale = geode::Mod::get()->getSettingValue<double>("cursor-scale");
    auto cursorFilter = geode::Mod::get()->getSettingValue<std::string>(
        "cursor-filter"
    );

    SpoutManager::get().updateFrameInterval(savedFPS);
    SpoutManager::get().setOutputResolution(savedResolution);
    SpoutManager::get().enableCursor(showCursor);
    fakecursor::setScale(cursorScale);
    fakecursor::setFilter(cursorFilter);

    listenForSettingChanges("output-fps", [](int fps) {
        SpoutManager::get().updateFrameInterval(fps);
    });

    listenForSettingChanges("output-resolution", [](CustomResolution resolution) {
        SpoutManager::get().setOutputResolution(resolution);
    });

    listenForSettingChanges("show-cursor", [](bool show) {
        SpoutManager::get().enableCursor(show);
    });

    listenForSettingChanges("cursor-scale", [](double scale) {
        fakecursor::setScale(scale);
    });

    listenForSettingChanges("cursor-filter", [](std::string const& filter) {
        fakecursor::setFilter(filter);
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
            SpoutManager::get().captureScreen(size.width, size.height, m_bShouldHideCursor);
        }
        CCEGLView::swapBuffers();
    }
};