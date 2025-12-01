// src/core/template_manifest.cpp
#include "cpp_hub/template_manifest.hpp"
#include "cpp_hub/util/string_utils.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <regex>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using nlohmann::json;
using cpp_hub::util::parse_bool;
using cpp_hub::util::trim;
using cpp_hub::util::to_lower;

namespace cpp_hub {

TemplateManifest load_manifest(const fs::path& manifestPath) {
    TemplateManifest manifest;

    std::ifstream in(manifestPath);
    if (!in) {
        throw std::runtime_error("Could not open manifest: " + manifestPath.string());
    }

    json j;
    try {
        in >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse hub-manifest.json: ") + e.what());
    }

    auto require_string = [&](const json& obj, const char* key) -> std::string {
        if (!obj.contains(key) || !obj[key].is_string()) {
            throw std::runtime_error(std::string("Manifest missing required string field '") +
                                     key + "'");
        }
        return obj[key].get<std::string>();
    };

    manifest.schemaVersion = require_string(j, "schema_version");
    manifest.id = require_string(j, "id");
    manifest.name = require_string(j, "name");
    manifest.version = require_string(j, "version");
    manifest.description = j.value("description", std::string{});

    if (!j.contains("variables") || !j["variables"].is_object()) {
        throw std::runtime_error("Manifest 'variables' must be an object.");
    }

    const json& vars = j["variables"];
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        VariableDef vd;
        vd.name = it.key();
        const json& v = it.value();

        if (!v.contains("type") || !v["type"].is_string()) {
            throw std::runtime_error("Variable '" + vd.name + "' missing 'type' field.");
        }
        std::string typeStr = v["type"].get<std::string>();
        if (typeStr == "string") {
            vd.type = VariableType::String;
        } else if (typeStr == "select") {
            vd.type = VariableType::Select;
        } else if (typeStr == "boolean") {
            vd.type = VariableType::Boolean;
        } else {
            throw std::runtime_error("Variable '" + vd.name + "' has unsupported type '" +
                                     typeStr + "'");
        }

        vd.prompt = v.value("prompt", vd.name + "?");

        if (v.contains("default")) {
            if (v["default"].is_string()) {
                vd.defaultValue = v["default"].get<std::string>();
            } else if (v["default"].is_boolean()) {
                vd.defaultValue = v["default"].get<bool>() ? "true" : "false";
            } else {
                vd.defaultValue = v["default"].dump();
            }
        }

        if (vd.type == VariableType::Select) {
            if (!v.contains("options") || !v["options"].is_array()) {
                throw std::runtime_error("Select variable '" + vd.name +
                                         "' must have 'options' array.");
            }
            for (const auto& o : v["options"]) {
                if (o.is_string()) {
                    vd.options.push_back(o.get<std::string>());
                }
            }
            if (vd.options.empty()) {
                throw std::runtime_error("Select variable '" + vd.name +
                                         "' has empty 'options' array.");
            }
        }

        if (v.contains("validation_regex") && v["validation_regex"].is_string()) {
            vd.validationRegex = v["validation_regex"].get<std::string>();
        }
        if (v.contains("error_message") && v["error_message"].is_string()) {
            vd.errorMessage = v["error_message"].get<std::string>();
        }

        manifest.variables.push_back(std::move(vd));
    }

    if (j.contains("hooks") && j["hooks"].is_object()) {
        const json& hooks = j["hooks"];
        if (hooks.contains("post_gen") && hooks["post_gen"].is_array()) {
            for (const auto& item : hooks["post_gen"]) {
                if (item.is_string()) {
                    manifest.postGenHooks.push_back(item.get<std::string>());
                }
            }
        }
    }

    return manifest;
}

static bool validate_string_variable(const VariableDef& vd, const std::string& value) {
    if (!vd.validationRegex.empty()) {
        try {
            std::regex re(vd.validationRegex);
            if (!std::regex_match(value, re)) {
                if (!vd.errorMessage.empty()) {
                    std::cerr << vd.errorMessage << "\n";
                } else {
                    std::cerr << "Value '" << value
                              << "' does not match required pattern.\n";
                }
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid validation_regex for variable '" << vd.name
                      << "': " << e.what() << "\n";
            // Treat as no validation, but log.
        }
    }
    return true;
}

static bool validate_select_variable(const VariableDef& vd, const std::string& value) {
    for (const auto& opt : vd.options) {
        if (opt == value) return true;
    }
    std::cerr << "Invalid value '" << value << "' for variable '" << vd.name
              << "'. Allowed options: ";
    for (size_t i = 0; i < vd.options.size(); ++i) {
        if (i > 0) std::cerr << ", ";
        std::cerr << vd.options[i];
    }
    std::cerr << "\n";
    return false;
}

bool collect_variables_interactively(
    const TemplateManifest& manifest,
    bool useDefaults,
    std::unordered_map<std::string, std::string>& outValues) {

    outValues.clear();

    for (const auto& vd : manifest.variables) {
        if (useDefaults) {
            if (vd.defaultValue.empty()) {
                std::cerr << "Variable '" << vd.name
                          << "' has no default; cannot use --defaults.\n";
                return false;
            }

            std::string value = vd.defaultValue;

            if (vd.type == VariableType::String) {
                if (!validate_string_variable(vd, value)) {
                    return false;
                }
            } else if (vd.type == VariableType::Select) {
                if (!validate_select_variable(vd, value)) {
                    return false;
                }
            } else if (vd.type == VariableType::Boolean) {
                bool b{};
                if (!parse_bool(value, b)) {
                    std::cerr << "Invalid default boolean value '" << value
                              << "' for variable '" << vd.name << "'.\n";
                    return false;
                }
                value = b ? "true" : "false";
            }

            outValues[vd.name] = value;
            continue;
        }

        while (true) {
            std::cout << vd.prompt;
            if (!vd.defaultValue.empty()) {
                std::cout << " [" << vd.defaultValue << "]";
            }
            if (vd.type == VariableType::Select) {
                std::cout << " (options: ";
                for (size_t i = 0; i < vd.options.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << vd.options[i];
                }
                std::cout << ")";
            }
            std::cout << "\n> " << std::flush;

            std::string input;
            if (!std::getline(std::cin, input)) {
                std::cerr << "Input aborted.\n";
                return false;
            }
            input = trim(input);

            std::string value;
            if (input.empty()) {
                value = vd.defaultValue;
            } else {
                value = input;
            }

            if (vd.type == VariableType::String) {
                if (value.empty()) {
                    std::cerr << "Value is required.\n";
                    continue;
                }
                if (!validate_string_variable(vd, value)) {
                    continue;
                }
                outValues[vd.name] = value;
                break;
            } else if (vd.type == VariableType::Select) {
                if (value.empty()) {
                    value = vd.defaultValue;
                }
                if (!validate_select_variable(vd, value)) {
                    continue;
                }
                outValues[vd.name] = value;
                break;
            } else if (vd.type == VariableType::Boolean) {
                if (value.empty()) {
                    value = vd.defaultValue;
                }
                bool b{};
                if (!parse_bool(value, b)) {
                    std::cerr << "Please enter yes/no, y/n, true/false.\n";
                    continue;
                }
                outValues[vd.name] = b ? "true" : "false";
                break;
            }
        }
    }

    return true;
}

} // namespace cpp_hub

