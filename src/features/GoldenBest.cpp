#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(GoldenBestLayer, PlayLayer) {
    struct Fields {
        bool m_isGold = false;
        float m_originalScale = 0.0f;
        float m_bestPercent = 0.0f;
        float m_highestThisAttempt = 0.0f;
    };

    CCLabelBMFont* getPercentageLabel() {
        if (m_percentageLabel) return m_percentageLabel;
        if (m_uiLayer) {
            return typeinfo_cast<CCLabelBMFont*>(m_uiLayer->getChildByID("percentage-label"));
        }
        return nullptr;
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (!level || level->isPlatformer()) return true;

        m_fields->m_bestPercent = static_cast<float>(level->m_normalPercent);
        m_fields->m_isGold = false;
        m_fields->m_highestThisAttempt = 0.0f;

        auto label = this->getPercentageLabel();
        if (label) {
            m_fields->m_originalScale = label->getScale();
        }

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (m_level && !m_level->isPlatformer()) {
            if (static_cast<float>(m_level->m_normalPercent) > m_fields->m_bestPercent) {
                m_fields->m_bestPercent = static_cast<float>(m_level->m_normalPercent);
            }
            if (m_fields->m_highestThisAttempt > m_fields->m_bestPercent) {
                m_fields->m_bestPercent = std::floor(m_fields->m_highestThisAttempt);
            }
        }

        m_fields->m_highestThisAttempt = 0.0f;

        if (m_fields->m_isGold) {
            m_fields->m_isGold = false;
            auto label = this->getPercentageLabel();
            if (label) {
                label->setFntFile("bigFont.fnt");
                if (m_fields->m_originalScale > 0.01f) {
                    label->setScale(m_fields->m_originalScale);
                }
            }
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!m_level || m_level->isPlatformer() || m_isPracticeMode || m_isTestMode) {
            return;
        }

        if (!Mod::get()->getSettingValue<bool>("golden-best")) {
            return;
        }

        float threshold = std::max(0.0f, m_fields->m_bestPercent);
        if (threshold >= 100.0f) {
            return;
        }

        float currentPercent = this->getCurrentPercent();
        if (currentPercent > m_fields->m_highestThisAttempt) {
            m_fields->m_highestThisAttempt = currentPercent;
        }

        bool isNewBest = false;
        if (threshold <= 0.0f) {
            isNewBest = (currentPercent >= 1.0f);
        } else {
            isNewBest = (currentPercent > threshold);
        }

        if (isNewBest && !m_fields->m_isGold) {
            auto label = this->getPercentageLabel();
            if (label) {
                m_fields->m_isGold = true;
                if (m_fields->m_originalScale <= 0.01f) {
                    m_fields->m_originalScale = label->getScale();
                    if (m_fields->m_originalScale <= 0.01f) {
                        m_fields->m_originalScale = 0.5f;
                    }
                }
                label->setFntFile("goldFont.fnt");
                label->setScale(m_fields->m_originalScale + 0.15f);
                label->setString(label->getString());
            }
        }
    }
};
