// src/core/registry.cpp
#include "cpp_hub/registry.hpp"
#include "cpp_hub/config.hpp"
#include "cpp_hub/util/fs.hpp"
#include "cpp_hub/util/process.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using nlohmann::json;

namespace cpp_hub {

Registry::Registry()
    : registryPath_(registry_path()), loaded_(false) {}

const fs::path& Registry::path() const {
    return registryPath_;
}

const std::string& Registry::name() const {
    return registryName_;
}

const std::map<std::string, TemplateInfo>& Registry::templates() const {
    return templates_;
}

const TemplateInfo* Registry::find_template(const std::string& id) const {
    auto it = templates_.find(id);
    if (it == templates_.end()) return nullptr;
    return &it->second;
}

void Registry::ensure_initialized() {
    if (!fs::exists(registryPath_)) {
        if (!util::ensure_directory(config_root())) {
            throw std::runtime_error("Failed to create config root directory.");
        }
        std::string cmd = "git clone \"" + default_registry_url() + "\" \"" +
                          registryPath_.string() + "\"";
        if (!util::run_command(cmd)) {
            throw std::runtime_error("Failed to clone registry from " + default_registry_url());
        }
    }
    reload();
}

void Registry::update() {
    if (!fs::exists(registryPath_)) {
        ensure_initialized();
        return;
    }
    std::string cmd = "git -C \"" + registryPath_.string() + "\" pull --ff-only";
    if (!util::run_command(cmd)) {
        throw std::runtime_error("Failed to update registry (git pull).");
    }
    reload();
}

void Registry::reload() {
    templates_.clear();
    loaded_ = false;
    load_index();
    loaded_ = true;
}

void Registry::load_index() {
    fs::path indexPath = registryPath_ / "index.json";
    std::ifstream in(indexPath);
    if (!in) {
        throw std::runtime_error("Could not open registry index: " + indexPath.string());
    }

    json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse registry index.json: ") + e.what());
    }

    if (j.contains("registry_name")) {
        registryName_ = j["registry_name"].get<std::string>();
    } else {
        registryName_.clear();
    }

    if (!j.contains("templates") || !j["templates"].is_object()) {
        throw std::runtime_error("Registry index.json missing 'templates' object.");
    }

    auto tmplObj = j["templates"];
    for (auto it = tmplObj.begin(); it != tmplObj.end(); ++it) {
        TemplateInfo info;
        const std::string key = it.key();
        const json& value = it.value();

        info.id = value.value("id", key);
        info.name = value.value("name", std::string{});
        info.description = value.value("description", std::string{});
        info.url = value.value("url", std::string{});
        info.buildSystem = value.value("build_system", std::string{});

        if (value.contains("tags") && value["tags"].is_array()) {
            for (const auto& t : value["tags"]) {
                if (t.is_string()) {
                    info.tags.push_back(t.get<std::string>());
                }
            }
        }

        if (info.id.empty() || info.url.empty()) {
            std::cerr << "Skipping template '" << key
                      << "' due to missing id or url.\n";
            continue;
        }

        templates_.emplace(info.id, std::move(info));
    }
}

} // namespace cpp_hub

