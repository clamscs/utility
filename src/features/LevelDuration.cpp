#include "../utils/Duration.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

class $modify(LevelDurationLayer, LevelInfoLayer) {
    struct Fields {
        CCLabelBMFont* m_durationLabel = nullptr;
    };

    CCNode* getLengthLabel() {
        CCNode* label = m_lengthLabel;
        if (!label) {
            label = this->getChildByID("length-label");
        }
        return label;
    }

    void updatePosition() {
        if (!m_fields->m_durationLabel) return;

        CCNode* lengthLabel = this->getLengthLabel();
        if (!lengthLabel || !lengthLabel->getParent()) return;

        CCRect bounds = lengthLabel->boundingBox();
        CCPoint worldBottomMid = lengthLabel->getParent()->convertToWorldSpace(
            ccp(bounds.getMidX(), bounds.getMinY())
        );

        CCPoint localPos = this->convertToNodeSpace(worldBottomMid);
        float offset = (m_fields->m_durationLabel->getScaledContentSize().height * 0.5f) + 1.5f;

        m_fields->m_durationLabel->setPosition({ localPos.x, localPos.y - offset });
    }

    void updateDurationDisplay() {
        if (!m_fields->m_durationLabel || !m_level) return;

        if (m_level->isPlatformer()) {
            if (Mod::get()->getSettingValue<bool>("better-platformer-time")) {
                if (m_level->m_timestamp > 0) {
                    double sec = static_cast<double>(m_level->m_timestamp) / 1000.0;
                    m_fields->m_durationLabel->setString(formatPlatformerTime(sec).c_str());
                    m_fields->m_durationLabel->setVisible(true);
                } else {
                    m_fields->m_durationLabel->setString("");
                    m_fields->m_durationLabel->setVisible(false);
                }
            } else {
                m_fields->m_durationLabel->setString("");
                m_fields->m_durationLabel->setVisible(false);
            }
            this->updatePosition();
            return;
        }

        bool showDuration = Mod::get()->getSettingValue<bool>("level-duration");
        int duration = calculateClassicDuration(m_level->m_levelString);

        if (showDuration && duration > 0) {
            m_fields->m_durationLabel->setString(formatDuration(duration).c_str());
            m_fields->m_durationLabel->setVisible(true);
        } else {
            m_fields->m_durationLabel->setString("");
            m_fields->m_durationLabel->setVisible(false);
        }

        this->updatePosition();
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (!level) return true;

        CCNode* lengthLabel = this->getLengthLabel();
        if (lengthLabel) {
            auto durationLabel = CCLabelBMFont::create("", "bigFont.fnt");
            durationLabel->setID("duration-label"_spr);
            durationLabel->setScale(0.26f);
            durationLabel->setOpacity(210);
            durationLabel->setColor({ 255, 255, 255 });
            durationLabel->setAnchorPoint({ 0.5f, 0.5f });

            this->addChild(durationLabel, lengthLabel->getZOrder() + 1);
            m_fields->m_durationLabel = durationLabel;

            this->updateDurationDisplay();
        }

        return true;
    }

    void levelDownloadFinished(GJGameLevel* level) {
        LevelInfoLayer::levelDownloadFinished(level);
        if (level == m_level) {
            this->updateDurationDisplay();
        }
    }
};
