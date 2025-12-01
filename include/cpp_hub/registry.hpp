// include/cpp_hub/registry.hpp
#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace cpp_hub {

struct TemplateInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string url;
    std::vector<std::string> tags;
    std::string buildSystem;
};

class Registry {
public:
    Registry();

    void ensure_initialized(); // clone if necessary, load index
    void update();             // git pull (or clone if missing), reload
    void reload();             // reload index.json (no git network access)

    const std::filesystem::path& path() const;
    const std::string& name() const;
    const std::map<std::string, TemplateInfo>& templates() const;
    const TemplateInfo* find_template(const std::string& id) const;

private:
    std::filesystem::path registryPath_;
    std::string registryName_;
    std::map<std::string, TemplateInfo> templates_;
    bool loaded_;

    void load_index();
};

} // namespace cpp_hub

