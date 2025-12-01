// include/cpp_hub/util/string_utils.hpp
#pragma once

#include <string>

namespace cpp_hub::util {

std::string to_lower(std::string s);
std::string trim(const std::string& s);
bool iequals(const std::string& a, const std::string& b);
bool icontains(const std::string& text, const std::string& sub);
bool parse_bool(const std::string& text, bool& out);

} // namespace cpp_hub::util

