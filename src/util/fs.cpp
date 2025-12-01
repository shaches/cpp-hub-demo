// src/util/fs.cpp
#include "cpp_hub/util/fs.hpp"
#include <cstdlib>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

namespace cpp_hub::util {

fs::path get_home_directory() {
#ifdef _WIN32
    if (const char* up = std::getenv("USERPROFILE")) {
        return fs::path(up);
    }
    if (const char* h = std::getenv("HOME")) {
        return fs::path(h);
    }
#else
    if (const char* h = std::getenv("HOME")) {
        return fs::path(h);
    }
#endif
    // Fallback
    return fs::current_path();
}

bool ensure_directory(const fs::path& dir) {
    std::error_code ec;
    if (fs::exists(dir, ec)) {
        if (!fs::is_directory(dir, ec)) {
            std::cerr << "Path exists but is not a directory: " << dir << "\n";
            return false;
        }
        return true;
    }
    if (!fs::create_directories(dir, ec)) {
        std::cerr << "Failed to create directory " << dir << ": " << ec.message() << "\n";
        return false;
    }
    return true;
}

} // namespace cpp_hub::util

