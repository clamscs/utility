#include "../utils/Duration.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/GJScoreCell.hpp>
#include <string_view>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cmath>

using namespace geode::prelude;

bool isBlacklistedId(std::string_view lowerId) {
    static constexpr std::string_view blacklist[] = {
        "id", "rank", "place", "pos", "attempt", "jump",
        "pass", "version", "rev", "daily", "weekly", "event",
        "time", "duration", "length", "percent", "title",
        "name", "author", "desc", "price", "cost"
    };
    for (auto b : blacklist) {
        if (lowerId.find(b) != std::string_view::npos) {
            return true;
        }
    }
    return false;
}

bool isWhitelistedId(std::string_view lowerId) {
    static constexpr std::string_view whitelist[] = {
        "download", "like", "object", "star", "moon",
        "diamond", "demon", "coin", "point", "score", "orb"
    };
    for (auto w : whitelist) {
        if (lowerId.find(w) != std::string_view::npos) {
            return true;
        }
    }
    return false;
}

void compactNodeLabels(CCNode* parent, float defaultScale = 0.5f) {
    if (!parent) return;

    for (auto child : CCArrayExt<CCNode*>(parent->getChildren())) {
        if (!child) continue;

        if (auto label = typeinfo_cast<CCLabelBMFont*>(child)) {
            std::string_view id = label->getID();
            std::string lowerId;
            lowerId.reserve(id.size());
            for (char c : id) {
                lowerId += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            if (isBlacklistedId(lowerId)) {
                compactNodeLabels(child, defaultScale);
                continue;
            }

            const char* cstr = label->getString();
            if (!cstr || cstr[0] == '\0') {
                compactNodeLabels(child, defaultScale);
                continue;
            }

            std::string_view sv(cstr);
            while (!sv.empty() && sv.front() == ' ') sv.remove_prefix(1);
            while (!sv.empty() && sv.back() == ' ') sv.remove_suffix(1);
            if (sv.empty()) {
                compactNodeLabels(child, defaultScale);
                continue;
            }

            if (sv.front() == '#' || sv.find(':') != std::string_view::npos || sv.find('%') != std::string_view::npos) {
                compactNodeLabels(child, defaultScale);
                continue;
            }

            if (sv.find('h') != std::string_view::npos || sv.find('m') != std::string_view::npos) {
                compactNodeLabels(child, defaultScale);
                continue;
            }
            if (sv.back() == 's' && sv.size() > 1 && !std::isdigit(static_cast<unsigned char>(sv[sv.size() - 2]))) {
                compactNodeLabels(child, defaultScale);
                continue;
            }

            bool negative = (sv.front() == '-');
            size_t startIdx = negative ? 1 : 0;
            std::string digits;
            digits.reserve(sv.size());
            bool pureNumber = true;
            bool hasComma = false;

            for (size_t i = startIdx; i < sv.size(); ++i) {
                char c = sv[i];
                if (c >= '0' && c <= '9') {
                    digits += c;
                } else if (c == ',') {
                    hasComma = true;
                } else {
                    pureNumber = false;
                    break;
                }
            }

            if (pureNumber && !digits.empty()) {
                bool isKnownStat = isWhitelistedId(lowerId);
                if (hasComma || isKnownStat) {
                    int64_t val = std::strtoll(digits.c_str(), nullptr, 10);
                    if (negative) val = -val;
                    if (std::abs(val) >= 1000) {
                        label->setString(formatCompactNumber(val).c_str());
                        label->setScale(defaultScale);
                    }
                }
            }
        }

        compactNodeLabels(child, defaultScale);
    }
}

class $modify(CompactNumbersInfoLayer, LevelInfoLayer) {
    void updateNumbers() {
        if (!Mod::get()->getSettingValue<bool>("compact-numbers")) return;

        if (m_level) {
            if (auto dl = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("downloads-label"))) {
                dl->setString(formatCompactNumber(m_level->m_downloads).c_str());
                dl->setScale(0.6f);
            }
            if (auto lk = typeinfo_cast<CCLabelBMFont*>(this->getChildByID("likes-label"))) {
                lk->setString(formatCompactNumber(m_level->m_likes).c_str());
                lk->setScale(0.6f);
            }
        }

        compactNodeLabels(this, 0.6f);
    }

    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;
        this->updateNumbers();
        return true;
    }

    void levelDownloadFinished(GJGameLevel* level) {
        LevelInfoLayer::levelDownloadFinished(level);
        if (level == m_level) {
            this->updateNumbers();
        }
    }
};

class $modify(CompactNumbersCell, LevelCell) {
    void loadFromLevel(GJGameLevel* level) {
        LevelCell::loadFromLevel(level);

        if (!level || !m_mainLayer) return;
        if (!Mod::get()->getSettingValue<bool>("compact-numbers")) return;

        if (auto dl = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("downloads-label"))) {
            dl->setString(formatCompactNumber(level->m_downloads).c_str());
            dl->setScale(0.4f);
        }
        if (auto lk = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("likes-label"))) {
            lk->setString(formatCompactNumber(level->m_likes).c_str());
            lk->setScale(0.4f);
        }

        compactNodeLabels(m_mainLayer, 0.4f);
    }
};

class $modify(CompactNumbersProfile, ProfilePage) {
    void loadPageFromUserInfo(GJUserScore* score) {
        ProfilePage::loadPageFromUserInfo(score);
        if (!Mod::get()->getSettingValue<bool>("compact-numbers")) return;

        compactNodeLabels(this, 0.6f);
    }
};

class $modify(CompactNumbersScoreCell, GJScoreCell) {
    void loadFromScore(GJUserScore* score) {
        GJScoreCell::loadFromScore(score);
        if (!Mod::get()->getSettingValue<bool>("compact-numbers")) return;

        compactNodeLabels(this, 0.4f);
    }
};
