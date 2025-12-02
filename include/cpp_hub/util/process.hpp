// include/cpp_hub/util/process.hpp
#pragma once

#include <filesystem>
#include <string>

namespace cpp_hub::util {

bool run_command(const std::string& cmd);
bool run_command_in_dir(const std::string& cmd, const std::filesystem::path& dir);

} // namespace cpp_hub::util