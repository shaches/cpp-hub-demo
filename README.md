# 🚀 cpp-hub: C++ Project Template Hub (AI-Generated Demo)

⚠️ **STABILITY NOTICE: CONCEPTUAL REFERENCE**

This repository is currently under **active construction** and is considered **highly volatile**. It is intended strictly as a **conceptual reference point**—a demonstration of what a C++ project template hub *could* have been—rather than a stable utility.

**Please be advised that this is not a production-ready tool.** Users should anticipate **incomplete functionality** and **frequent breaking changes**.

---

## 💡 Overview

`cpp-hub` is a small **C++20 CLI** designed to significantly reduce the "Day Zero" friction associated with starting new C++ projects. It acts as a project generator that leverages remote templates.

### Core Functionality

1.  **Discovery:** Finds available project templates through a local registry (a Git repository containing an `index.json`).
2.  **Caching:** Clones template repositories into a local cache (`~/.cpp-hub/cache`).
3.  **Generation:** Creates new projects by applying simple `{{variable}}` substitutions within template file contents and paths.
4.  **Hooks:** Optionally runs template-defined **post-generation hooks** (e.g., `git init`, `git add .`).

---

## ✨ Features (Minimum Viable Product - MVP)

### Registry-Backed Discovery

* The **Registry** is a Git repository (default URL is hard-coded in `config.cpp`).
* The local clone lives under: `~/.cpp-hub/registry`.
* Templates are cataloged in an `index.json` file (including `id`, `name`, `url`, `tags`, `build system`, etc.).
* `cpp-hub update` executes a `git pull` on the local registry clone.

### Template Cache

* Template repositories are cloned under: `~/.cpp-hub/cache/<template-id>`.
* Templates are **reused** on subsequent runs, eliminating the need to re-clone every time.

### Template Manifests

Each template repository **must** contain a `hub-manifest.json` at its root. This manifest defines the template's behavior:

* **Basic metadata:** `schema_version`, `id`, `name`, `version`, `description`.
* **`variables`:** Defines inputs requested from the user, supporting:
    * `string` (with optional `validation_regex` and `error_message`).
    * `select` (with predefined `options` and a `default`).
    * `boolean` (`y/n`, `yes/no`, `true/false`, case-insensitive).
* **`hooks.post_gen`:** A list of shell commands to optionally run after the project is generated.

### Simple Templating

Variable substitution (`{{variable_name}}`) is applied to:

* **File contents**
* **Relative paths / filenames**

This mechanism applies to all regular files under the template root, excluding:
* The `.git` directory.
* The `hub-manifest.json` file.

---

## 💻 Commands

| Command | Description |
| :--- | :--- |
| `cpp-hub new <template-id> [--defaults]` | Generates a new project from a registered template. |
| `cpp-hub new --git <url> [--branch <name>] [--defaults]` | Generates a new project directly from a Git URL. |
| `cpp-hub search <query>` | Searches the registry for templates matching the query. |
| `cpp-hub list [--tag <tag>]` | Lists all available templates (optionally filtered by tag). |
| `cpp-hub update` | Updates the local template registry via `git pull`. |
| `cpp-hub validate <path>` | Validates the `hub-manifest.json` within a template path. |
| `cpp-hub version` | Displays the `cpp-hub` version information. |

---

## 🛠️ Requirements

### Toolchain
* **Language:** C++20 (or newer)
* **Build system:** CMake $\geq$ 3.16

### Dependencies
* `nlohmann::json` (available via package managers or `vcpkg`, etc.)

### Runtime Tools
* `git` must be available on your `PATH` (used internally via `std::system`).

---

## ⚙️ Building

Assuming `nlohmann_json` is installed on your system so that `find_package(nlohmann_json CONFIG REQUIRED)` works:

```bash
# Configure the build system
cmake -S . -B build

# Build the project executable
cmake --build build