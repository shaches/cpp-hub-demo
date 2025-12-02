// include/cpp_hub/util/fs.hpp
#pragma once

#include <filesystem>

namespace cpp_hub::util {

std::filesystem::path get_home_directory();
bool ensure_directory(const std::filesystem::path& dir);

} // namespace cpp_hub::util