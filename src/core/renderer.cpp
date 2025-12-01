// src/core/renderer.cpp
#include "cpp_hub/renderer.hpp"
#include "cpp_hub/util/string_utils.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

namespace cpp_hub {

static std::string apply_substitutions(
    std::string text,
    const std::unordered_map<std::string, std::string>& values) {

    for (const auto& [key, value] : values) {
        const std::string token = "{{" + key + "}}";
        std::string::size_type pos = 0;
        while ((pos = text.find(token, pos)) != std::string::npos) {
            text.replace(pos, token.size(), value);
            pos += value.size();
        }
    }
    return text;
}

static bool path_contains_git_dir(const fs::path& p, const fs::path& root) {
    fs::path cur = p;
    while (cur != root && !cur.empty()) {
        if (cur.filename() == ".git") {
            return true;
        }
        cur = cur.parent_path();
    }
    return false;
}

bool render_template(
    const fs::path& templateRoot,
    const fs::path& targetRoot,
    const std::unordered_map<std::string, std::string>& values) {

    std::error_code ec;
    if (fs::exists(targetRoot, ec)) {
        std::cerr << "Target path already exists: " << targetRoot << "\n";
        return false;
    }
    if (!fs::create_directories(targetRoot, ec) && ec) {
        std::cerr << "Failed to create target directory " << targetRoot
                  << ": " << ec.message() << "\n";
        return false;
    }

    try {
        fs::recursive_directory_iterator it(templateRoot, fs::directory_options::skip_permission_denied), end;
        for (; it != end; ++it) {
            const fs::path srcPath = it->path();

            if (path_contains_git_dir(srcPath, templateRoot)) {
                if (it->is_directory() && srcPath.filename() == ".git") {
                    it.disable_recursion_pending();
                }
                continue;
            }

            fs::path rel = fs::relative(srcPath, templateRoot, ec);
            if (ec) {
                std::cerr << "Failed to compute relative path for " << srcPath
                          << ": " << ec.message() << "\n";
                return false;
            }

            // Skip manifest itself
            if (rel.filename() == "hub-manifest.json") {
                if (it->is_directory()) {
                    it.disable_recursion_pending();
                }
                continue;
            }

            std::string relStr = rel.generic_string();
            relStr = apply_substitutions(relStr, values);
            fs::path destPath = targetRoot / fs::path(relStr);

            if (it->is_directory()) {
                if (!fs::create_directories(destPath, ec) && ec) {
                    std::cerr << "Failed to create directory " << destPath
                              << ": " << ec.message() << "\n";
                    return false;
                }
                continue;
            }

            if (!it->is_regular_file()) {
                continue;
            }

            fs::create_directories(destPath.parent_path(), ec);

            std::ifstream in(srcPath, std::ios::binary);
            if (!in) {
                std::cerr << "Failed to open template file " << srcPath << "\n";
                return false;
            }

            std::ostringstream buffer;
            buffer << in.rdbuf();
            std::string content = buffer.str();
            std::string rendered = apply_substitutions(content, values);

            std::ofstream out(destPath, std::ios::binary);
            if (!out) {
                std::cerr << "Failed to create file " << destPath << "\n";
                return false;
            }
            out << rendered;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error while rendering template: " << e.what() << "\n";
        return false;
    }

    return true;
}

} // namespace cpp_hub

