// src/core/config.cpp
#include "cpp_hub/config.hpp"
#include "cpp_hub/util/fs.hpp"

namespace fs = std::filesystem;

namespace cpp_hub {

std::string default_registry_url() {
    // Change this to your real registry URL
    return "https://github.com/example/cpp-hub-registry.git";
}

fs::path config_root() {
    fs::path home = util::get_home_directory();
    return home / ".cpp-hub";
}

fs::path registry_path() {
    return config_root() / "registry";
}

fs::path cache_root() {
    return config_root() / "cache";
}

std::string version() {
#ifdef CPP_HUB_VERSION
    return CPP_HUB_VERSION;
#else
    return "0.1.0";
#endif
}

} // namespace cpp_hub