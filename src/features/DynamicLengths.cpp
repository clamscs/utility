#include "../utils/Duration.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <string_view>

using namespace geode::prelude;

class $modify(DynamicLengthsLayer, LevelInfoLayer) {
    struct Fields {
        CCLabelBMFont* m_lengthLabel = nullptr;
        float m_originalScale = 1.0f;
        std::string m_originalString = "";
    };

    CCLabelBMFont* findLengthLabel() {
        if (m_lengthLabel) return m_lengthLabel;
        return typeinfo_cast<CCLabelBMFont*>(this->getChildByID("length-label"));
    }

    void alignDurationLabel() {
        if (!m_fields->m_lengthLabel || !m_fields->m_lengthLabel->getParent()) return;

        auto durationLabel = this->getChildByID("duration-label"_spr);
        if (!durationLabel) return;

        CCRect bounds = m_fields->m_lengthLabel->boundingBox();
        CCPoint worldBottomMid = m_fields->m_lengthLabel->getParent()->convertToWorldSpace(
            ccp(bounds.getMidX(), bounds.getMinY())
        );

        CCPoint localPos = this->convertToNodeSpace(worldBottomMid);
        float offset = (durationLabel->getScaledContentSize().height * 0.5f) + 1.5f;

        durationLabel->setPosition({ localPos.x, localPos.y - offset });
    }

    void updateLengthDisplay() {
        if (!m_fields->m_lengthLabel || !m_level || m_level->isPlatformer()) return;

        bool dynamicLengths = Mod::get()->getSettingValue<bool>("dynamic-lengths");
        int duration = calculateClassicDuration(m_level->m_levelString);

        if (dynamicLengths && duration >= 120) {
            std::string newLength = getExtendedLengthString(duration);
            m_fields->m_lengthLabel->setString(newLength.c_str());

            size_t xCount = newLength.length() - 1;
            if (xCount > 3) {
                float scaleFactor = 3.5f / static_cast<float>(xCount);
                m_fields->m_lengthLabel->setScale(m_fields->m_originalScale * scaleFactor);
            } else {
                m_fields->m_lengthLabel->setScale(m_fields->m_originalScale);
            }
        } else if (!m_fields->m_originalString.empty()) {
            m_fields->m_lengthLabel->setString(m_fields->m_originalString.c_str());
            m_fields->m_lengthLabel->setScale(m_fields->m_originalScale);
        }

        this->alignDurationLabel();
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (!level || level->isPlatformer()) return true;

        CCLabelBMFont* lengthLabel = this->findLengthLabel();
        if (lengthLabel) {
            m_fields->m_lengthLabel = lengthLabel;
            m_fields->m_originalScale = lengthLabel->getScale();
            if (const char* str = lengthLabel->getString()) {
                m_fields->m_originalString = str;
            }

            this->updateLengthDisplay();
        }

        return true;
    }

    void levelDownloadFinished(GJGameLevel* level) {
        LevelInfoLayer::levelDownloadFinished(level);
        if (level == m_level && !level->isPlatformer()) {
            this->updateLengthDisplay();
        }
    }
};

class $modify(DynamicLengthsCell, LevelCell) {
    struct Fields {
        float m_originalScale = 0.0f;
    };

    CCLabelBMFont* findCellLengthLabel() {
        if (!m_mainLayer) return nullptr;

        if (auto label = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("length-label"))) {
            return label;
        }

        for (auto child : CCArrayExt<CCNode*>(m_mainLayer->getChildren())) {
            if (auto bm = typeinfo_cast<CCLabelBMFont*>(child)) {
                std::string_view s = bm->getString();
                if (s == "Tiny" || s == "Short" || s == "Medium" || s == "Long") return bm;
                if (s.size() >= 2 && s.size() <= 8 && s.back() == 'L') {
                    bool allX = true;
                    for (size_t i = 0; i < s.size() - 1; ++i) {
                        if (s[i] != 'X') {
                            allX = false;
                            break;
                        }
                    }
                    if (allX) return bm;
                }
            }
        }
        return nullptr;
    }

    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        if (!level) return;

        if (level->isPlatformer()) {
            if (Mod::get()->getSettingValue<bool>("better-platformer-time")) {
                if (auto lengthLabel = this->findCellLengthLabel()) {
                    const char* str = lengthLabel->getString();
                    double sec = parsePlatformerSeconds(str);
                    if (sec > 0.0) {
                        lengthLabel->setString(formatPlatformerTime(sec).c_str());
                        float maxWidth = 28.0f;
                        float rawWidth = lengthLabel->getContentSize().width;
                        if (rawWidth > 0.0f) {
                            float scale = lengthLabel->getScale();
                            if (rawWidth * scale > maxWidth) {
                                lengthLabel->setScale(maxWidth / rawWidth);
                            }
                        }
                    }
                }
            }
            return;
        }

        if (!Mod::get()->getSettingValue<bool>("dynamic-lengths")) return;

        auto actualLevel = level;
        if (actualLevel->m_levelString.empty()) {
            auto glm = GameLevelManager::sharedState();
            if (glm) {
                auto saved = glm->getSavedLevel(level->m_levelID);
                if (saved && !saved->m_levelString.empty()) {
                    actualLevel = saved;
                }
            }
        }

        if (actualLevel->m_levelString.empty()) return;

        int duration = calculateClassicDuration(actualLevel->m_levelString);
        if (duration < 120) return;

        CCLabelBMFont* lengthLabel = this->findCellLengthLabel();
        if (!lengthLabel) return;

        if (m_fields->m_originalScale <= 0.01f) {
            m_fields->m_originalScale = lengthLabel->getScale();
            if (m_fields->m_originalScale <= 0.01f) {
                m_fields->m_originalScale = 0.4f;
            }
        }

        std::string newLength = getExtendedLengthString(duration);
        if (!newLength.empty()) {
            lengthLabel->setString(newLength.c_str());

            float maxWidth = 28.0f;
            float rawWidth = lengthLabel->getContentSize().width;
            if (rawWidth > 0.0f) {
                float targetScale = m_fields->m_originalScale;
                if (rawWidth * targetScale > maxWidth) {
                    targetScale = maxWidth / rawWidth;
                }
                lengthLabel->setScale(targetScale);
            }
        }
    }
};
