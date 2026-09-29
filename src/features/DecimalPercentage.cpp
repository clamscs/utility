#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cstdio>
#include <algorithm>

using namespace geode::prelude;

class $modify(DecimalPercentageLayer, PlayLayer) {
    CCLabelBMFont* getPercentageLabel() {
        if (m_percentageLabel) return m_percentageLabel;
        if (m_uiLayer) {
            return typeinfo_cast<CCLabelBMFont*>(m_uiLayer->getChildByID("percentage-label"));
        }
        return nullptr;
    }

    void updateCustomPercentage() {
        if (!m_level || m_level->isPlatformer()) return;
        if (!Mod::get()->getSettingValue<bool>("decimal-percentage")) return;

        auto label = this->getPercentageLabel();
        if (!label) return;

        double currentPercent = static_cast<double>(this->getCurrentPercent());
        if (currentPercent < 0.0) currentPercent = 0.0;
        if (currentPercent > 100.0) currentPercent = 100.0;

        int decimals = static_cast<int>(Mod::get()->getSettingValue<int64_t>("decimal-places"));
        decimals = std::clamp(decimals, 0, 10);

        char buffer[64];
        if (decimals > 0) {
            std::snprintf(buffer, sizeof(buffer), "%.*f%%", decimals, currentPercent);
        } else {
            std::snprintf(buffer, sizeof(buffer), "%d%%", static_cast<int>(currentPercent));
        }

        label->setString(buffer);
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();
        this->updateCustomPercentage();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        this->updateCustomPercentage();
    }
};
