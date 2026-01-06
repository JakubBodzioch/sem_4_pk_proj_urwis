#include "KeywordService.hpp"
#include <fstream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include "json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

std::vector<std::string> KeywordService::load_keywords(const std::string& filepath) {
    std::vector<std::string> keywords;
    if (!fs::exists(filepath)) return keywords;

    std::ifstream file(filepath);
    if (!file.is_open()) return keywords;

    try {
        json j;
        file >> j;
        if (!j.is_array()) return keywords;

        for (const auto& item : j) {
            if (item.is_string())
                keywords.push_back(item.get<std::string>());
        }
    }
    catch (...) {
        // ignorujemy uszkodzone pliki
    }
    return keywords;
}

std::vector<std::string> KeywordService::extract_keywords(const std::string& text) {
    std::vector<std::string> matched_keywords;
    std::string lower_text = text;
    std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);

    static const std::string keywords_path = "stuff/keywords.json";
    std::vector<std::string> keywords = load_keywords(keywords_path);

    for (const auto& key : keywords) {
        std::string escaped_key = std::regex_replace(key, std::regex(R"([.^$|()\\[\]{}*+?])"), R"(\\$&)");
        std::string pattern = "\\b" + escaped_key + "\\b";
        std::regex re(pattern, std::regex_constants::icase);

        if (std::regex_search(lower_text, re)) {
            matched_keywords.push_back(key);
        }
    }
    return matched_keywords;
}
