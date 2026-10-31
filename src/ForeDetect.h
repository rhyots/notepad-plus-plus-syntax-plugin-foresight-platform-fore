#pragma once
#include <cstddef>
#include <string>

namespace fore {
inline constexpr std::size_t DetectionLimit = 128 * 1024;
// Only Normal Text documents with .txt or no extension (including new tabs).
bool autoDetectEligible(const std::wstring& path, bool normalText);
bool looksLikeFore(const std::string& text);
}
