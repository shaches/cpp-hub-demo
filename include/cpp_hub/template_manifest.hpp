// include/cpp_hub/template_manifest.hpp
#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace cpp_hub {

enum class VariableType {
    String,
    Select,
    Boolean
};

struct VariableDef {
    VariableType type;
    std::string name;
    std::string prompt;
    std::string defaultValue;
    std::vector<std::string> options;
    std::string validationRegex;
    std::string errorMessage;
};

struct TemplateManifest {
    std::string schemaVersion;
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::vector<VariableDef> variables;
    std::vector<std::string> postGenHooks;
};

TemplateManifest load_manifest(const std::filesystem::path& manifestPath);

// Interactively (or via defaults) collect variable values.
// Returns false on error or if user aborts.
bool collect_variables_interactively(
    const TemplateManifest& manifest,
    bool useDefaults,
    std::unordered_map<std::string, std::string>& outValues);

} // namespace cpp_hub

