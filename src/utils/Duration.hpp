#pragma once

#include <string>
#include <cstdint>

std::string formatDuration(int totalSeconds);
std::string getExtendedLengthString(int duration);
int calculateClassicDuration(const std::string& levelString);
std::string formatPlatformerTime(double totalSeconds);
double parsePlatformerSeconds(const char* str);
std::string formatCompactNumber(int64_t num);
