#pragma once

#include "Geode/cocos/cocoa/CCObject.h"
#include "includes.hpp"

#include <Geode/Result.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <memory>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/TextInput.hpp>

struct CustomResolution {
    bool enabled = false;
    int width = 1920;
    int height = 1080;

    bool operator==(CustomResolution const& other) const {
        return enabled == other.enabled && 
            width == other.width && 
            height == other.height;
    }

    CustomResolution() = default;
    CustomResolution(bool enabled, int width, int height) : enabled(enabled), width(width), height(height) {}

    CustomResolution(CustomResolution const&) = default;
};

template <>
struct matjson::Serialize<CustomResolution> {
    static matjson::Value toJson(CustomResolution const& res) {
        matjson::Value value;
        value["enabled"] = res.enabled;
        value["width"] = res.width;
        value["height"] = res.height;
        return value;
    }

    static Result<CustomResolution> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(bool e, value["enabled"].asBool());
        GEODE_UNWRAP_INTO(int w, value["width"].as<int>());
        GEODE_UNWRAP_INTO(int h, value["height"].as<int>());
        return Ok(CustomResolution{e, w, h});
    }
};

class ResolutionSettingV3 : public SettingBaseValueV3<CustomResolution> {
public:
    static Result<std::shared_ptr<SettingV3>> parse(
        std::string const& key,
        std::string const& modID,
        matjson::Value const& json
    );

    SettingNodeV3* createNode(float width) override;
};

template <>
struct SettingTypeForValueType<CustomResolution> {
    using SettingType = ResolutionSettingV3;
};

class ResolutionSettingNodeV3 : public SettingValueNodeV3<ResolutionSettingV3> {
protected:
    CCMenuItemToggler* m_toggle;
    TextInput* m_widthInput;
    TextInput* m_heightInput;

    bool init(std::shared_ptr<ResolutionSettingV3> setting, float width);
    void onToggle(CCObject*);
    void onTextChange(CCNode* invoker);
    void updateState(CCNode* invoker) override;

public:
    static ResolutionSettingNodeV3* create(std::shared_ptr<ResolutionSettingV3> setting, float width);
};