// src/cli/cli.cpp
#include "cpp_hub/cli.hpp"

#include "cpp_hub/config.hpp"
#include "cpp_hub/registry.hpp"
#include "cpp_hub/renderer.hpp"
#include "cpp_hub/template_manifest.hpp"
#include "cpp_hub/util/fs.hpp"
#include "cpp_hub/util/process.hpp"
#include "cpp_hub/util/string_utils.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace fs = std::filesystem;
using cpp_hub::util::icontains;
using cpp_hub::util::iequals;
using cpp_hub::util::parse_bool;
using cpp_hub::util::trim;

namespace cpp_hub::cli {

static int handle_version(const std::vector<std::string>& args);
static int handle_update(const std::vector<std::string>& args);
static int handle_search(const std::vector<std::string>& args);
static int handle_list(const std::vector<std::string>& args);
static int handle_validate(const std::vector<std::string>& args);
static int handle_new(const std::vector<std::string>& args);
static int handle_new_from_registry(const std::vector<std::string>& args);
static int handle_new_from_git(const std::vector<std::string>& args);
static int run_generation(const fs::path& templateRepoPath, bool useDefaults);

void print_usage(std::ostream& os) {
    os << "Usage:\n"
       << "  cpp-hub new <template-id> [--defaults]\n"
       << "  cpp-hub new --git <url> [--branch <name>] [--defaults]\n"
       << "  cpp-hub search <query>\n"
       << "  cpp-hub list [--tag <tag>]\n"
       << "  cpp-hub update\n"
       << "  cpp-hub validate <path>\n"
       << "  cpp-hub version\n";
}

int run(int argc, char** argv) {
    if (argc < 2) {
        print_usage(std::cerr);
        return 1;
    }

    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc - 1));
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    const std::string& cmd = args[0];
    std::vector<std::string> rest(args.begin() + 1, args.end());

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_usage(std::cout);
        return 0;
    } else if (cmd == "version") {
        return handle_version(rest);
    } else if (cmd == "update") {
        return handle_update(rest);
    } else if (cmd == "search") {
        return handle_search(rest);
    } else if (cmd == "list") {
        return handle_list(rest);
    } else if (cmd == "validate") {
        return handle_validate(rest);
    } else if (cmd == "new") {
        return handle_new(rest);
    } else {
        std::cerr << "Unknown command: " << cmd << "\n";
        print_usage(std::cerr);
        return 1;
    }
}

static int handle_version(const std::vector<std::string>& /*args*/) {
    std::cout << "cpp-hub version " << cpp_hub::version() << "\n";
    fs::path regPath = cpp_hub::registry_path();
    std::cout << "Registry path: " << regPath << " (";
    if (fs::exists(regPath)) {
        std::cout << "exists";
    } else {
        std::cout << "not initialized";
    }
    std::cout << ")\n";

    if (fs::exists(regPath)) {
        try {
            cpp_hub::Registry reg;
            reg.reload();
            if (!reg.name().empty()) {
                std::cout << "Registry name: " << reg.name() << "\n";
            }
        } catch (...) {
            // Best-effort; ignore errors here.
        }
    }

    return 0;
}

static int handle_update(const std::vector<std::string>& /*args*/) {
    try {
        cpp_hub::Registry reg;
        reg.update();
        std::cout << "Registry updated. Templates: " << reg.templates().size() << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to update registry: " << e.what() << "\n";
        return 1;
    }
}

static void print_template_table_header() {
    std::cout << std::left
              << std::setw(20) << "ID"
              << std::setw(24) << "NAME"
              << std::setw(14) << "BUILD SYSTEM"
              << "DESCRIPTION"
              << "\n";
    std::cout << std::string(20 + 24 + 14 + 40, '-') << "\n";
}

static int handle_search(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "search: missing <query>\n";
        print_usage(std::cerr);
        return 1;
    }

    std::string query;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) query += ' ';
        query += args[i];
    }

    try {
        cpp_hub::Registry reg;
        reg.ensure_initialized();
        const auto& templates = reg.templates();

        std::vector<const TemplateInfo*> matches;
        for (const auto& [id, t] : templates) {
            bool matched = false;
            if (icontains(t.id, query) ||
                icontains(t.name, query) ||
                icontains(t.description, query)) {
                matched = true;
            } else {
                for (const auto& tag : t.tags) {
                    if (icontains(tag, query)) {
                        matched = true;
                        break;
                    }
                }
            }
            if (matched) {
                matches.push_back(&t);
            }
        }

        if (matches.empty()) {
            std::cout << "No templates matched query: " << query << "\n";
            return 0;
        }

        print_template_table_header();
        for (const auto* t : matches) {
            std::cout << std::left
                      << std::setw(20) << t->id.substr(0, 19)
                      << std::setw(24) << t->name.substr(0, 23)
                      << std::setw(14) << t->buildSystem.substr(0, 13)
                      << t->description
                      << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "search failed: " << e.what() << "\n";
        return 1;
    }
}

static int handle_list(const std::vector<std::string>& args) {
    std::string tagFilter;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--tag") {
            if (i + 1 >= args.size()) {
                std::cerr << "list: --tag requires a value\n";
                return 1;
            }
            tagFilter = args[i + 1];
            i++;
        } else {
            std::cerr << "list: unknown option '" << args[i] << "'\n";
            return 1;
        }
    }

    try {
        cpp_hub::Registry reg;
        reg.ensure_initialized();
        const auto& templates = reg.templates();

        std::vector<const TemplateInfo*> matches;
        for (const auto& [id, t] : templates) {
            if (tagFilter.empty()) {
                matches.push_back(&t);
            } else {
                bool hasTag = false;
                for (const auto& tag : t.tags) {
                    if (iequals(tag, tagFilter)) {
                        hasTag = true;
                        break;
                    }
                }
                if (hasTag) {
                    matches.push_back(&t);
                }
            }
        }

        if (matches.empty()) {
            if (tagFilter.empty()) {
                std::cout << "No templates in registry.\n";
            } else {
                std::cout << "No templates with tag '" << tagFilter << "'.\n";
            }
            return 0;
        }

        print_template_table_header();
        for (const auto* t : matches) {
            std::cout << std::left
                      << std::setw(20) << t->id.substr(0, 19)
                      << std::setw(24) << t->name.substr(0, 23)
                      << std::setw(14) << t->buildSystem.substr(0, 13)
                      << t->description
                      << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "list failed: " << e.what() << "\n";
        return 1;
    }
}

static int handle_validate(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "validate: missing <path>\n";
        print_usage(std::cerr);
        return 1;
    }

    fs::path root = args[0];
    fs::path manifestPath = root / "hub-manifest.json";

    try {
        TemplateManifest manifest = load_manifest(manifestPath);
        (void)manifest;
        std::cout << "Template manifest is valid.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Template manifest is invalid: " << e.what() << "\n";
        return 1;
    }
}

static int handle_new(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "new: missing <template-id> or --git <url>\n";
        print_usage(std::cerr);
        return 1;
    }
    if (args[0] == "--git") {
        return handle_new_from_git(args);
    }
    return handle_new_from_registry(args);
}

static int handle_new_from_registry(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "new: missing <template-id>\n";
        return 1;
    }

    std::string templateId = args[0];
    bool useDefaults = false;

    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--defaults") {
            useDefaults = true;
        } else {
            std::cerr << "new: unknown option '" << args[i] << "'\n";
            return 1;
        }
    }

    try {
        cpp_hub::Registry reg;
        reg.ensure_initialized();
        const TemplateInfo* info = reg.find_template(templateId);
        if (!info) {
            std::cerr << "Template not found in registry: " << templateId << "\n";
            return 1;
        }

        fs::path cacheRoot = cpp_hub::cache_root();
        if (!util::ensure_directory(cacheRoot)) {
            std::cerr << "Failed to create cache directory.\n";
            return 1;
        }

        fs::path tplPath = cacheRoot / info->id;

        if (!fs::exists(tplPath)) {
            std::string cmd = "git clone \"" + info->url + "\" \"" + tplPath.string() + "\"";
            if (!util::run_command(cmd)) {
                std::cerr << "Failed to clone template repository: " << info->url << "\n";
                return 1;
            }
        } else {
            std::cout << "Using cached template at " << tplPath << "\n";
        }

        return run_generation(tplPath, useDefaults);
    } catch (const std::exception& e) {
        std::cerr << "new failed: " << e.what() << "\n";
        return 1;
    }
}

static int handle_new_from_git(const std::vector<std::string>& args) {
    // args[0] == "--git"
    if (args.size() < 2) {
        std::cerr << "new --git: missing <url>\n";
        return 1;
    }
    std::string url = args[1];
    std::string branch;
    bool useDefaults = false;

    for (size_t i = 2; i < args.size(); ++i) {
        if (args[i] == "--branch") {
            if (i + 1 >= args.size()) {
                std::cerr << "--branch requires a name\n";
                return 1;
            }
            branch = args[i + 1];
            ++i;
        } else if (args[i] == "--defaults") {
            useDefaults = true;
        } else {
            std::cerr << "new --git: unknown option '" << args[i] << "'\n";
            return 1;
        }
    }

    fs::path cacheRoot = cpp_hub::cache_root();
    if (!util::ensure_directory(cacheRoot)) {
        std::cerr << "Failed to create cache directory.\n";
        return 1;
    }

    std::hash<std::string> hasher;
    std::string dirName = "git-" + std::to_string(hasher(url + branch));
    fs::path tplPath = cacheRoot / dirName;

    if (!fs::exists(tplPath)) {
        std::string cmd = "git clone \"" + url + "\" \"" + tplPath.string() + "\"";
        if (!util::run_command(cmd)) {
            std::cerr << "Failed to clone template repository: " << url << "\n";
            return 1;
        }
    } else {
        std::cout << "Using cached repository at " << tplPath << "\n";
    }

    if (!branch.empty()) {
        std::string cmd = "git -C \"" + tplPath.string() + "\" checkout \"" + branch + "\"";
        if (!util::run_command(cmd)) {
            std::cerr << "Failed to checkout branch '" << branch << "'.\n";
            return 1;
        }
    }

    return run_generation(tplPath, useDefaults);
}

static int run_generation(const fs::path& templateRepoPath, bool useDefaults) {
    fs::path manifestPath = templateRepoPath / "hub-manifest.json";
    TemplateManifest manifest;

    try {
        manifest = load_manifest(manifestPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load manifest: " << e.what() << "\n";
        return 1;
    }

    std::unordered_map<std::string, std::string> values;
    if (!collect_variables_interactively(manifest, useDefaults, values)) {
        return 1;
    }

    std::string defaultName;
    auto it = values.find("project_name");
    if (it != values.end() && !it->second.empty()) {
        defaultName = it->second;
    } else if (!manifest.id.empty()) {
        defaultName = manifest.id;
    } else {
        defaultName = "cpp-project";
    }

    fs::path defaultTarget = fs::current_path() / defaultName;

    std::cout << "Target directory [" << defaultTarget.string() << "]: " << std::flush;
    std::string input;
    if (!std::getline(std::cin, input)) {
        std::cerr << "Aborted.\n";
        return 1;
    }
    input = trim(input);

    fs::path targetPath;
    if (input.empty()) {
        targetPath = defaultTarget;
    } else {
        targetPath = fs::path(input);
        if (targetPath.is_relative()) {
            targetPath = fs::current_path() / targetPath;
        }
    }

    if (fs::exists(targetPath)) {
        std::cerr << "Error: target path already exists: " << targetPath << "\n";
        return 1;
    }

    // 1) Render base template
    if (!render_template(templateRepoPath, targetPath, values)) {
        return 1;
    }

    // 2) Apply overlays (if any) on top, depending on collected variables
    for (const auto& rule : manifest.overlays) {
        auto vit = values.find(rule.variable);
        if (vit == values.end()) {
            continue;
        }
        if (vit->second != rule.equalsValue) {
            continue;
        }

        fs::path overlayRoot = templateRepoPath / rule.path;
        if (!fs::exists(overlayRoot)) {
            std::cerr << "Warning: overlay path does not exist: " << overlayRoot << "\n";
            continue;
        }

        std::cout << "Applying overlay for " << rule.variable
                  << " == " << rule.equalsValue
                  << " from " << overlayRoot << "\n";

        if (!render_template(overlayRoot, targetPath, values, /*allowExisting*/ true)) {
            std::cerr << "Failed to render overlay from " << overlayRoot << "\n";
            return 1;
        }
    }

    std::cout << "Project generated at: " << targetPath << "\n";

    if (!manifest.postGenHooks.empty()) {
        for (const auto& cmd : manifest.postGenHooks) {
            std::cout << "This template wants to run: \"" << cmd << "\"\n";
            std::cout << "Run it? [Y/n] " << std::flush;

            std::string answer;
            if (!std::getline(std::cin, answer)) {
                std::cerr << "Aborted.\n";
                return 1;
            }
            answer = trim(answer);
            bool runCmd = answer.empty();
            if (!answer.empty()) {
                bool b{};
                if (parse_bool(answer, b)) {
                    runCmd = b;
                } else {
                    // If ambiguous, default to "no" for safety.
                    runCmd = false;
                }
            }

            if (runCmd) {
                if (!util::run_command_in_dir(cmd, targetPath)) {
                    std::cerr << "Hook command failed (continuing): " << cmd << "\n";
                }
            } else {
                std::cout << "Skipped: " << cmd << "\n";
            }
        }
    }

    return 0;
}

} // namespace cpp_hub::cli

