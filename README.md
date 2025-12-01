# cpp-hub (AI-generated demo)

> ⚠️ **Note:** This is an **AI-generated demo implementation** of a C++ project template hub CLI.  
> It is intended as a starting point / reference, **not** a production-ready tool.

`cpp-hub` is a small C++20 CLI that reduces “Day Zero” friction for C++ projects by:

- Discovering templates via a **local registry** (a Git repo with `index.json`).
- Cloning template repositories into a local **cache**.
- Generating new projects by applying simple `{{variable}}` substitutions in files and paths.
- Optionally running template-defined **post-generation hooks** (e.g., `git init`, `git add .`).

---

## Features (MVP)

- **Registry-backed discovery**
  - Registry is a Git repo (default URL is hard-coded in `config.cpp`).
  - Local clone lives under: `~/.cpp-hub/registry`
  - Templates are described in `index.json` (id, name, url, tags, build system, etc.).
  - `cpp-hub update` performs `git pull` on the registry.

- **Template cache**
  - Template repos are cloned under: `~/.cpp-hub/cache/<template-id>`  
  - Reused on subsequent runs (no need to reclone every time).

- **Template manifests**
  - Each template repo contains a `hub-manifest.json` at its root.
  - Defines:
    - Basic metadata: `schema_version`, `id`, `name`, `version`, `description`.
    - `variables`:
      - `string` (with optional `validation_regex` and `error_message`)
      - `select` (with `options` and `default`)
      - `boolean` (`y/n`, `yes/no`, `true/false`, case-insensitive)
    - `hooks.post_gen`: a list of shell commands to optionally run after generation.

- **Simple templating**
  - Replaces `{{variable_name}}` in:
    - File contents
    - Relative paths / filenames
  - Applies to all regular files under the template root, excluding:
    - `.git` directory
    - `hub-manifest.json`

- **Commands**
  - `cpp-hub new <template-id> [--defaults]`
  - `cpp-hub new --git <url> [--branch <name>] [--defaults]`
  - `cpp-hub search <query>`
  - `cpp-hub list [--tag <tag>]`
  - `cpp-hub update`
  - `cpp-hub validate <path>`
  - `cpp-hub version`

---

## Requirements

- **Language:** C++20 (or newer)
- **Build system:** CMake ≥ 3.16
- **Dependencies:**
  - [`nlohmann::json`](https://github.com/nlohmann/json) (available via package managers or vcpkg, etc.)
- **Runtime tools:**
  - `git` available on `PATH` (used via `std::system`)

---

## Building

Assuming `nlohmann_json` is installed so that `find_package(nlohmann_json CONFIG REQUIRED)` works:

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build
