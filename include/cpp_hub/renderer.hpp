// include/cpp_hub/renderer.hpp
#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace cpp_hub {

// Copy directory tree from templateRoot to targetRoot, applying {{var}} replacements.
//
// If allowExisting is false (default), the targetRoot must not exist yet.
// If allowExisting is true, files and directories are merged into an existing tree,
// overriding files if they already exist.
bool render_template(
    const std::filesystem::path& templateRoot,
    const std::filesystem::path& targetRoot,
    const std::unordered_map<std::string, std::string>& values,
    bool allowExisting = false);

} // namespace cpp_hub