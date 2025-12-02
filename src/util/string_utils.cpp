// src/util/string_utils.cpp
#include "cpp_hub/util/string_utils.hpp"
#include <algorithm>
#include <cctype>

namespace cpp_hub::util {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string trim(const std::string& s) {
    if (s.empty()) return s;
    size_t start = 0;
    size_t end = s.size() - 1;

    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) {
        --end;
    }
    return s.substr(start, end - start + 1);
}

bool iequals(const std::string& a, const std::string& b) {
    return to_lower(a) == to_lower(b);
}

bool icontains(const std::string& text, const std::string& sub) {
    std::string t = to_lower(text);
    std::string s = to_lower(sub);
    return t.find(s) != std::string::npos;
}

bool parse_bool(const std::string& text, bool& out) {
    std::string v = to_lower(trim(text));
    if (v == "y" || v == "yes" || v == "true" || v == "1") {
        out = true;
        return true;
    }
    if (v == "n" || v == "no" || v == "false" || v == "0") {
        out = false;
        return true;
    }
    return false;
}

} // namespace cpp_hub::util