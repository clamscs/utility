#include "Duration.hpp"
#include <Geode/Geode.hpp>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string_view>
#include <cstdio>
#include <cstdlib>

using namespace geode::prelude;

std::string formatDuration(int totalSeconds) {
    if (totalSeconds <= 0) return "";

    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    std::string result;
    if (hours > 0) {
        result += std::to_string(hours) + "h ";
        result += std::to_string(minutes) + "m ";
        result += std::to_string(seconds) + "s";
    } else if (minutes > 0) {
        result += std::to_string(minutes) + "m ";
        result += std::to_string(seconds) + "s";
    } else {
        result += std::to_string(seconds) + "s";
    }
    return result;
}

std::string formatPlatformerTime(double totalSeconds) {
    if (totalSeconds < 0.0) totalSeconds = 0.0;
    int totalSec = static_cast<int>(totalSeconds);
    int hours = totalSec / 3600;
    int minutes = (totalSec % 3600) / 60;
    int seconds = totalSec % 60;

    std::string result;
    if (hours > 0) {
        result += std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    } else if (minutes > 0) {
        result += std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    } else {
        result += std::to_string(seconds) + "s";
    }
    return result;
}

double parsePlatformerSeconds(const char* str) {
    if (!str || str[0] == '\0') return 0.0;

    std::string_view sv(str);

    if (sv.find(':') != std::string_view::npos) {
        int h = 0, m = 0;
        double s = 0.0;
        size_t firstColon = sv.find(':');
        size_t secondColon = sv.find(':', firstColon + 1);
        if (secondColon != std::string_view::npos) {
            std::sscanf(str, "%d:%d:%lf", &h, &m, &s);
            return h * 3600.0 + m * 60.0 + s;
        } else {
            std::sscanf(str, "%d:%lf", &m, &s);
            return m * 60.0 + s;
        }
    }

    if (sv.find('h') != std::string_view::npos || sv.find('m') != std::string_view::npos) {
        double total = 0.0;
        size_t hPos = sv.find('h');
        if (hPos != std::string_view::npos) {
            size_t start = sv.rfind(' ', hPos);
            start = (start == std::string_view::npos) ? 0 : start + 1;
            total += std::strtod(str + start, nullptr) * 3600.0;
        }
        size_t mPos = sv.find('m');
        if (mPos != std::string_view::npos) {
            size_t start = sv.rfind(' ', mPos);
            start = (start == std::string_view::npos) ? 0 : start + 1;
            total += std::strtod(str + start, nullptr) * 60.0;
        }
        size_t sPos = sv.find('s');
        if (sPos != std::string_view::npos) {
            size_t start = sv.rfind(' ', sPos);
            start = (start == std::string_view::npos) ? 0 : start + 1;
            total += std::strtod(str + start, nullptr);
        }
        if (total > 0.0) return total;
    }

    char* end = nullptr;
    double val = std::strtod(str, &end);
    return (val > 0.0) ? val : 0.0;
}

std::string formatCompactNumber(int64_t num) {
    bool negative = num < 0;
    uint64_t n = negative ? static_cast<uint64_t>(-num) : static_cast<uint64_t>(num);

    double val = 0.0;
    const char* suffix = "";

    if (n >= 999'950'000) {
        val = static_cast<double>(n) / 1'000'000'000.0;
        suffix = "B";
    } else if (n >= 999'950) {
        val = static_cast<double>(n) / 1'000'000.0;
        suffix = "M";
    } else if (n >= 1'000) {
        val = static_cast<double>(n) / 1'000.0;
        suffix = "K";
    } else {
        return (negative ? "-" : "") + std::to_string(n);
    }

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f", val);

    std::string str = buf;
    if (str.size() >= 2 && str.compare(str.size() - 2, 2, ".0") == 0) {
        str.erase(str.size() - 2);
    }

    return (negative ? "-" : "") + str + suffix;
}

std::string getExtendedLengthString(int duration) {
    if (duration < 120) return "";

    int xCount = 1;
    if (duration >= 1500) {
        xCount = 6;
    } else if (duration >= 900) {
        xCount = 5;
    } else if (duration >= 600) {
        xCount = 4;
    } else if (duration >= 420) {
        xCount = 3;
    } else if (duration >= 240) {
        xCount = 2;
    } else {
        xCount = 1;
    }

    return std::string(xCount, 'X') + "L";
}

struct SpeedChange {
    float x;
    float speed;
};

int calculateClassicDuration(const std::string& levelString) {
    if (levelString.empty()) return 0;

    std::string data = ZipUtils::decompressString(levelString, false, 0);
    if (data.empty()) data = levelString;

    std::stringstream ss(data);
    std::string token;
    float startSpeed = 311.58f;
    std::vector<SpeedChange> portals;
    float maxX = 0.0f;

    while (std::getline(ss, token, ';')) {
        if (token.empty()) continue;

        if (token.find("kA") != std::string::npos || token.find("kS") != std::string::npos) {
            std::stringstream headerStream(token);
            std::string key, val;
            while (std::getline(headerStream, key, ',') && std::getline(headerStream, val, ',')) {
                if (key == "kA4") {
                    int speedIndex = std::atoi(val.c_str());
                    if (speedIndex == 1) startSpeed = 251.16f;
                    else if (speedIndex == 0) startSpeed = 311.58f;
                    else if (speedIndex == 2) startSpeed = 387.42f;
                    else if (speedIndex == 3) startSpeed = 468.0f;
                    else if (speedIndex == 4) startSpeed = 576.0f;
                }
            }
            continue;
        }

        std::stringstream objStream(token);
        std::string key, val;
        int id = 0;
        float x = 0.0f;

        while (std::getline(objStream, key, ',') && std::getline(objStream, val, ',')) {
            if (key == "1") id = std::atoi(val.c_str());
            else if (key == "2") x = std::strtof(val.c_str(), nullptr);
        }

        if (x > maxX) maxX = x;

        if (id == 200) portals.push_back({ x, 251.16f });
        else if (id == 201) portals.push_back({ x, 311.58f });
        else if (id == 202) portals.push_back({ x, 387.42f });
        else if (id == 203) portals.push_back({ x, 468.0f });
        else if (id == 1334) portals.push_back({ x, 576.0f });
    }

    if (maxX <= 0.0f) return 0;

    std::sort(portals.begin(), portals.end(), [](const SpeedChange& a, const SpeedChange& b) {
        return a.x < b.x;
    });

    float currentX = 0.0f;
    float currentSpeed = startSpeed;
    float totalTime = 0.0f;

    for (const auto& portal : portals) {
        if (portal.x > maxX) break;
        if (portal.x > currentX) {
            totalTime += (portal.x - currentX) / currentSpeed;
            currentX = portal.x;
        }
        currentSpeed = portal.speed;
    }

    if (maxX > currentX) {
        totalTime += (maxX - currentX) / currentSpeed;
    }

    return static_cast<int>(totalTime);
}
