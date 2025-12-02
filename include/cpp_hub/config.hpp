// include/cpp_hub/config.hpp
#pragma once

#include <filesystem>
#include <string>

namespace cpp_hub {

std::string default_registry_url();

std::filesystem::path config_root();
std::filesystem::path registry_path();
std::filesystem::path cache_root();

std::string version();

} // namespace cpp_hub