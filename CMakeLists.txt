cmake_minimum_required(VERSION 3.16)
project(cpp-hub VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# You may need to install nlohmann_json (e.g. via vcpkg, package manager, or add_subdirectory)
find_package(nlohmann_json CONFIG REQUIRED)

set(SOURCES
    src/main.cpp
    src/cli/cli.cpp
    src/core/config.cpp
    src/core/registry.cpp
    src/core/template_manifest.cpp
    src/core/renderer.cpp
    src/util/fs.cpp
    src/util/process.cpp
    src/util/string_utils.cpp
)

add_executable(cpp-hub ${SOURCES})

target_include_directories(cpp-hub
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cpp-hub
    PRIVATE
        nlohmann_json::nlohmann_json
)

target_compile_definitions(cpp-hub
    PRIVATE
        CPP_HUB_VERSION="${PROJECT_VERSION}"
)

