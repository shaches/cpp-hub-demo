// include/cpp_hub/renderer.hpp
#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace cpp_hub {

// Copy directory tree from templateRoot to targetRoot, applying {{var}} replacements.
bool render_template(
    const std::filesystem::path& templateRoot,
    const std::filesystem::path& targetRoot,
    const std::unordered_map<std::string, std::string>& values);

} // namespace cpp_hub

