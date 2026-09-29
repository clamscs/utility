#include "../utils/Duration.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string_view>

using namespace geode::prelude;

class $modify(BetterPlatformerTimeLayer, PlayLayer) {
    struct Fields {
        double m_time = 0.0;
    };

    CCLabelBMFont* getTimerLabel() {
        if (m_percentageLabel) return m_percentageLabel;
        if (m_uiLayer) {
            return typeinfo_cast<CCLabelBMFont*>(m_uiLayer->getChildByID("percentage-label"));
        }
        return nullptr;
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (level && level->isPlatformer()) {
            m_fields->m_time = 0.0;
            if (Mod::get()->getSettingValue<bool>("better-platformer-time")) {
                if (auto label = this->getTimerLabel()) {
                    label->setVisible(true);
                    label->setString("0s");
                }
            }
        }
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (m_level && m_level->isPlatformer()) {
            m_fields->m_time = 0.0;
            if (Mod::get()->getSettingValue<bool>("better-platformer-time")) {
                if (auto label = this->getTimerLabel()) {
                    label->setVisible(true);
                    label->setString("0s");
                }
            }
        }
    }

    void updateTimerDisplay() {
        if (!m_level || !m_level->isPlatformer()) return;
        if (!Mod::get()->getSettingValue<bool>("better-platformer-time")) return;

        auto label = this->getTimerLabel();
        if (!label) return;

        label->setVisible(true);

        const char* currentText = label->getString();
        double sec = 0.0;
        if (currentText && currentText[0] != '\0' && !std::string_view(currentText).contains('%')) {
            sec = parsePlatformerSeconds(currentText);
        }
        if (sec <= 0.0) {
            sec = m_fields->m_time;
        }

        label->setString(formatPlatformerTime(sec).c_str());
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        this->updateTimerDisplay();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (m_level && m_level->isPlatformer()) {
            m_fields->m_time += static_cast<double>(dt);
            this->updateTimerDisplay();
        }
    }
};
