#include "ResolutionSetting.hpp"
#include "Geode/ui/TextInput.hpp"
#include "Geode/utils/general.hpp"
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <memory>
#include <string>

Result<std::shared_ptr<SettingV3>> ResolutionSettingV3::parse(
    std::string const& key,
    std::string const& modID,
    matjson::Value const& json
) {
    auto res = std::make_shared<ResolutionSettingV3>();
    auto root = checkJson(json, "ResolutionSettingV3");

    res->parseBaseProperties(key, modID, root);
    root.checkUnknownKeys();

    return root.ok(std::static_pointer_cast<SettingV3>(res));
}

SettingNodeV3* ResolutionSettingV3::createNode(float width) {
    return ResolutionSettingNodeV3::create(
        std::static_pointer_cast<ResolutionSettingV3>(shared_from_this()),
        width
    );
}

bool ResolutionSettingNodeV3::init(
    std::shared_ptr<ResolutionSettingV3> setting,
    float width
) {
    if (!SettingValueNodeV3::init(setting, width)) return false;

    m_toggle = CCMenuItemToggler::createWithStandardSprites(
        this, 
        menu_selector(ResolutionSettingNodeV3::onToggle), 
        .7f
    );
    m_toggle->m_notClickable = true;

    m_widthInput = TextInput::create(60.f, "W");
    m_widthInput->setScale(0.7f);
    m_widthInput->setCommonFilter(CommonFilter::Int);
    m_widthInput->setCallback(
        [this](std::string const& text) { this->onTextChange(m_widthInput); }
    );

    auto separatorLabel = CCLabelBMFont::create("x", "chatFont.fnt");

    m_heightInput = TextInput::create(60.f, "H");
    m_heightInput->setScale(0.7f);
    m_heightInput->setCommonFilter(CommonFilter::Int);
    m_heightInput->setCallback(
        [this](std::string const& text) { this->onTextChange(m_widthInput); }
    );

    auto menu = this->getButtonMenu();
    menu->addChild(m_toggle);
    menu->addChildAtPosition(m_widthInput, Anchor::Center);
    menu->addChild(separatorLabel);
    menu->addChildAtPosition(m_heightInput, Anchor::Center);

    menu->setLayout(RowLayout::create());
    menu->setContentWidth(120.f);

    this->updateState(nullptr);
    return true;
}

void ResolutionSettingNodeV3::onToggle(CCObject*) {
    auto value = this->getValue();
    value.enabled = !m_toggle->isToggled();
    this->setValue(value, m_toggle);
    this->markChanged(m_toggle);
}

void ResolutionSettingNodeV3::onTextChange(CCNode* invoker) {
    auto value = this->getValue();
    value.width = geode::utils::numFromString<int>(m_widthInput->getString()).unwrapOrDefault();
    value.height = geode::utils::numFromString<int>(m_heightInput->getString()).unwrapOrDefault();

    this->setValue(value, nullptr);
    this->markChanged(invoker);
}

void ResolutionSettingNodeV3::updateState(CCNode* invoker) {
    SettingValueNodeV3::updateState(invoker);

    auto value = this->getValue();

    if (m_toggle && m_widthInput && m_heightInput) {
        m_toggle->toggle(value.enabled);

        if (invoker != m_widthInput) {
            m_widthInput->setString(std::to_string(value.width));
        }
        if (invoker != m_heightInput) {
            m_heightInput->setString(std::to_string(value.height));
        }

        m_widthInput->setEnabled(value.enabled);
        m_heightInput->setEnabled(value.enabled);
    }
}

ResolutionSettingNodeV3* ResolutionSettingNodeV3::create(std::shared_ptr<ResolutionSettingV3> setting, float width) {
    auto ret = new ResolutionSettingNodeV3();
    if (ret->init(setting, width)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}