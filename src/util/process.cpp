// src/util/process.cpp
#include "cpp_hub/util/process.hpp"
#include <cstdlib>
#include <iostream>

namespace fs = std::filesystem;

namespace cpp_hub::util {

bool run_command(const std::string& cmd) {
    std::cout << "$ " << cmd << "\n";
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        std::cerr << "Command failed with code " << rc << ": " << cmd << "\n";
        return false;
    }
    return true;
}

bool run_command_in_dir(const std::string& cmd, const fs::path& dir) {
    std::error_code ec;
    fs::path old = fs::current_path(ec);
    if (ec) {
        std::cerr << "Failed to get current directory: " << ec.message() << "\n";
        return false;
    }

    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        std::cerr << "Directory does not exist: " << dir << "\n";
        return false;
    }

    fs::current_path(dir, ec);
    if (ec) {
        std::cerr << "Failed to change directory to " << dir << ": " << ec.message() << "\n";
        return false;
    }

    std::cout << "(in " << dir << ")\n";
    int rc = std::system(cmd.c_str());

    fs::current_path(old, ec);
    if (rc != 0) {
        std::cerr << "Command failed with code " << rc << ": " << cmd << "\n";
        return false;
    }
    return true;
}

} // namespace cpp_hub::util

