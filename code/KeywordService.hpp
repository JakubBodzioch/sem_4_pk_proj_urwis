#pragma once
#include <string>
#include <vector>

class KeywordService {
public:
    static std::vector<std::string> load_keywords(const std::string& filepath);
    static std::vector<std::string> extract_keywords(const std::string& text);
};
